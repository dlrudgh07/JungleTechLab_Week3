#include "FName.h"
#include <shared_mutex>
#include <vector>
#include <array>
#include <memory>
#include <string_view>
#include <algorithm>
#include <cctype>

// ============================================================================
// 공통 설정 및 데이터 구조체
// ============================================================================
using FNameEntryId = uint32;
constexpr FNameEntryId INVALID_NAME_ID = 0xFFFFFFFF;

// 16384(2^14)칸짜리 블록을 만들기 위한 비트 연산 마스크
constexpr uint32 BLOCK_SHIFT = 14;			
constexpr uint32 BLOCK_SIZE = 1 << BLOCK_SHIFT; // 16384
constexpr uint32 OFFSET_MASK = BLOCK_SIZE - 1;

// 실제 문자열과 원본 해시값을 보관하는 데이터 구조
struct FNameEntry
{
	std::string StringData; // 원본 대소문자가 유지된 문자열 (ToString 출력용)
	uint32 HashValue;     // 소문자 변환 후 계산된 해시값 (빠른 비교용)
};




// ============================================================================
// 구역 1: FNamePoolData (문자열 데이터 창고)
// 역할: 실제 문자열 메모리를 할당하고, 메모리 단편화 없이 관리하며 ID를 발급합니다.
// ============================================================================
class FNamePoolData
{
private:
	// 고정 크기(16384)의 블록(Array)들을 관리하는 리스트입니다.
	// std::unique_ptr를 사용하여 블록이 메모리상에서 절대 이동하지 않도록 보장합니다.
	std::vector<std::unique_ptr<std::array<FNameEntry, BLOCK_SIZE>>> Blocks;

	uint32 CurrentBlockIndex = 0;			// 현재 문자를 채워넣고 있는 블록 번호
	uint32 CurrentOffset = BLOCK_SIZE;	// 현재 블록 안에서 몇 번째 칸인지 (0 ~ 16383)
	std::mutex PoolAllocationLock;			// "완전히 새로운 문자"가 등록될 때만 걸리는 전역 락

public:
	// 새로운 문자열을 창고에 저장하고, 물리적 위치 정보가 담긴 4바이트 ID를 발급합니다.
	FNameEntryId AllocateEntry(std::string_view InputString, uint32 Hash)
	{
		// 이 구역은 "새로운 문자를 쓸 때"만 들어오므로 일반 mutex로 보호합니다.
		std::lock_guard<std::mutex> Lock(PoolAllocationLock);

		// 1. 현재 블록이 꽉 차면(16384칸 도달) 아예 블록이 없으면 새 블록을 생성합니다.
		if (CurrentOffset >= BLOCK_SIZE)
		{
			Blocks.push_back(std::make_unique<std::array<FNameEntry, BLOCK_SIZE>>());
			CurrentBlockIndex = static_cast<uint32>(Blocks.size()) - 1;
			CurrentOffset = 0; // 새 블록이므로 오프셋은 0부터 시작
		}

		// 2. 현재 빈칸에 문자열과 해시값을 저장합니다.
		(*Blocks[CurrentBlockIndex])[CurrentOffset] = { std::string(InputString), Hash };

		// 3. ID 생성 마법: [블록 번호(상위 18비트)] + [블록 내 위치(하위 14비트)]
		FNameEntryId NewId = (CurrentBlockIndex << BLOCK_SHIFT) | CurrentOffset;
		++CurrentOffset; // 다음 칸으로 이동

		return NewId;
	}

	// ID를 해석하여 실제 문자열 데이터가 있는 포인터를 즉시(O(1)) 반환합니다.
	FNameEntry* GetEntry(FNameEntryId Id)
	{
		uint32 BlockIndex = Id >> BLOCK_SHIFT; // 상위 비트 추출
		uint32 Offset = Id & OFFSET_MASK;      // 하위 14비트 추출
		return &(*Blocks[BlockIndex])[Offset];
	}
};

// 전역변수로 저장소 관리
static FNamePoolData GNamePoolData;





// ============================================================================
// 구역 2: FNameShard (해시 검색대 및 락 관리)
// 역할: 멀티스레딩 병목을 막고(Lock Sharding), 선형 탐사로 해시 충돌을 해결합니다.
// ============================================================================
class FNameShard
{
private:
	// 이 Shard 전용 락. 읽기(Shared)와 쓰기(Unique)를 모두 통제합니다.
	mutable std::shared_mutex ShardLock;

	// 선형 탐사(Linear Probing)를 위한 해시 테이블. 실제 데이터 대신 ID만 가집니다.
	std::vector<FNameEntryId> ProbingTable;

	// 리사이징(Rehashing) 시점을 알기 위해 현재 들어있는 데이터 개수를 추적합니다.
	size_t CurrentCount = 0;

	// 테이블이 꽉 찼을 때 크기를 2배로 늘리고 데이터를 재배치하는 함수
	void Rehash(FNamePoolData& GlobalPool)
	{
		size_t OldCapacity = ProbingTable.size();
		size_t NewCapacity = OldCapacity * 2; // 항상 2의 거듭제곱 유지

		// 1. 2배 크기의 새로운 테이블 생성 및 초기화
		std::vector<FNameEntryId> NewTable(NewCapacity, INVALID_NAME_ID);

		// 2. 기존 테이블의 유효한 ID들을 새 테이블로 이사 (Re-hashing)
		for (size_t CurrentIndex = 0; CurrentIndex < OldCapacity; ++CurrentIndex)
		{
			FNameEntryId Id = ProbingTable[CurrentIndex];
			if (Id != INVALID_NAME_ID)
			{
				// ID로 전역 창고에서 해시값을 다시 가져와 새 위치를 계산합니다.
				FNameEntry* Entry = GlobalPool.GetEntry(Id);
				size_t NewIndex = Entry->HashValue & (NewCapacity - 1);

				// 새 테이블에서 선형 탐사로 빈자리 찾기
				while (NewTable[NewIndex] != INVALID_NAME_ID)
				{
					NewIndex = (NewIndex + 1) & (NewCapacity - 1);
				}

				NewTable[NewIndex] = Id;
			}
		}

		// 3. 기존 테이블을 새 테이블로 교체
		ProbingTable = std::move(NewTable);
	}

public:
	FNameShard()
	{
		// 엔진 시작 시 충돌을 최소화하기 위해 2의 거듭제곱 크기로 초기화합니다.
		ProbingTable.resize(1024, INVALID_NAME_ID);
	}

	// 문자열이 이미 테이블에 있는지 확인하고, 없다면 새로 등록한 뒤 ID를 반환합니다.
	FNameEntryId FindOrAdd(std::string_view InputString, uint32_t InputHash, FNamePoolData& GlobalPool)
	{
		size_t Capacity = ProbingTable.size();
		size_t StartIndex = InputHash & (Capacity - 1); // % 대신 비트 연산으로 고속 인덱스 계산

		// -------------------------------------------------------------
		// Step A: [Read Lock] 공유 락을 걸고 이미 있는 글자인지 캐시 히트 검사
		// -------------------------------------------------------------
		{
			std::shared_lock<std::shared_mutex> ReadLock(ShardLock);
			size_t Index = StartIndex;

			while (ProbingTable[Index] != INVALID_NAME_ID)
			{
				FNameEntry* Entry = GlobalPool.GetEntry(ProbingTable[Index]);
				if (Entry->HashValue == InputHash && Entry->StringData == InputString)
				{
					return ProbingTable[Index];
				}
				Index = (Index + 1) & (Capacity - 1);
			}
		}

		// -------------------------------------------------------------
		// Step B: [Write Lock] 발견하지 못했다면 독점 락을 걸고 새로 등록
		// -------------------------------------------------------------
		{
			std::unique_lock<std::shared_mutex> WriteLock(ShardLock);

			// Double-check Locking: 내가 WriteLock을 기다리는 찰나에 다른 스레드가 먼저 등록했을 수 있습니다.
			size_t Index = StartIndex;
			while (ProbingTable[Index] != INVALID_NAME_ID)
			{
				FNameEntry* Entry = GlobalPool.GetEntry(ProbingTable[Index]);
				if (Entry->HashValue == InputHash && Entry->StringData == InputString)
				{
					return ProbingTable[Index];
				}
				Index = (Index + 1) & (Capacity - 1);
			}

			// 빈자리를 찾았더라도, 테이블이 75% 이상 찼다면 리사이징을 먼저 수행합니다.
			if (CurrentCount >= Capacity * 0.75f)
			{
				Rehash(GlobalPool);

				// 테이블 크기가 2배로 변했으므로, 인덱스 계산에 쓰이는 변수들을 반드시 갱신해야 합니다.
				Capacity = ProbingTable.size();
				StartIndex = InputHash & (Capacity - 1);

				// 새로운 테이블 기준으로 다시 빈자리를 찾습니다.
				Index = StartIndex;
				while (ProbingTable[Index] != INVALID_NAME_ID)
				{
					Index = (Index + 1) & (Capacity - 1);
				}
			}

			// 진짜로 아무도 등록하지 않은 새로운 글자이므로, 전역 창고(Pool)에 문자를 넘기고 새 ID를 발급받습니다.
			FNameEntryId NewId = GlobalPool.AllocateEntry(InputString, InputHash);

			// 내 Shard의 선형 탐사 테이블 빈칸에 발급받은 ID를 기록하고 카운트를 올립니다.
			ProbingTable[Index] = NewId;
			CurrentCount++; 

			return NewId;
		}
	}
};

// 전역변수로 저장소 관리
constexpr size_t MAX_SHARD_COUNT = 1024;
static FNameShard GNameShards[MAX_SHARD_COUNT];


// ============================================================================
// 구역 3: FName 클래스 멤버 함수 구현부 (유저 인터페이스)
// 역할: 외부에서 헤더를 통해 접근하는 함수들. 해시값을 계산하고 Shard로 작업을 토스합니다.
// ============================================================================
FName::FName(std::string_view InputString)
{
	// 1. 대소문자 무시(Case-Insensitive)를 위해 임시 문자열을 전부 소문자로 변환합니다.
	std::string LowerStr(InputString);
	std::transform(LowerStr.begin(), LowerStr.end(), LowerStr.begin(),
		[](unsigned char c) { return std::tolower(c); });

	// 2. 소문자로 변환된 문자열을 바탕으로 32비트 해시값을 뽑아냅니다.
	uint32_t Hash = static_cast<uint32_t>(std::hash<std::string>{}(LowerStr));

	// 3. 해시값을 이용해 이 문자가 1024개의 Shard 중 어디로 갈지 결정합니다.
	size_t ShardIndex = Hash % MAX_SHARD_COUNT;

	// 4. 해당 Shard에게 문자열과 해시값을 던져주고 4바이트 ID를 받아와 저장합니다.
	this->ID = GNameShards[ShardIndex].FindOrAdd(InputString, Hash, GNamePoolData);
}

// O(1) 초고속 비교: 문자열을 한 글자씩 비교하지 않고 4바이트 정수만 비교합니다.
bool FName::operator==(const FName& Other) const
{
	return this->ID == Other.ID;
}

// 화면에 출력하거나 로깅할 때, ID를 들고 창고(Pool)에 가서 원본 문자열을 꺼내옵니다.
std::string FName::ToString() const
{
	return GNamePoolData.GetEntry(this->ID)->StringData;
}
