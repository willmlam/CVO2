#ifndef ARE_workspace_HXX_INCLUDED
#define ARE_workspace_HXX_INCLUDED

#include <stdlib.h>

#if defined WINDOWS || _WINDOWS
#include <windows.h>
#endif // WINDOWS

#include "Explanation.hxx"
#include "Function.hxx"

namespace ARE { class ARP ; }
namespace ARE { class Function ; }

#ifndef ARP_nInfinity
#define ARP_nInfinity (-std::numeric_limits<double>::infinity())
#endif // ARP_nInfinity
#ifndef ARP_pInfinity
#define ARP_pInfinity (std::numeric_limits<double>::infinity())
#endif // ARP_pInfinity

namespace ARE
{

class Workspace
{
protected :
	bool _IsValid ; // indicates whether workspace was constructed correctly
public :
	inline bool IsValid(void) const { return _IsValid ; }

protected :
	// we assume that the problem does NOT belong to the workspace unless otherwise specified
	bool _ProblemBelongsToWorkspace ;
	ARE::ARP *_Problem ;
public :
	inline bool & ProblemBelongsToWorkspace(void) { return _ProblemBelongsToWorkspace ; }
	inline ARE::ARP *Problem(void) const { return _Problem ; }

	// **************************************************************************************************
	// error codes occuring during computation
	// **************************************************************************************************

protected :
	bool _HasFatalError ;
	ARE::Explanation *_ExplanationList ;
public :
	inline bool HasFatalError(void) const { return _HasFatalError ; }
	inline void SetFatalError(void) { _HasFatalError = true ; }
	void AddErrorExplanation(ARE::Function *f) ;
	void AddExplanation(ARE::Explanation & E) ;
	bool HasErrorExplanation(void) ;

	// **************************************************************************************************
	// External Memory BE
	// **************************************************************************************************

protected :
	// This mutex is used to synchronize access to Function-Table-Block related data within this workspace.
	// In particular, it is used :
	// 1) protect access to _CurrentDiskMemorySpaceCached/_MaximumDiskMemorySpaceCached variables of this workspace.
	// 2) _FTBsInMemory ptr (list) of functions in this workspace.
	// 3) indirectly, the users list of each FTB.
	// 4) maintain/update the _BucketFunctionBlockComputationResult arrays in each bucket
	ARE::utils::RecursiveMutex _FTBMutex ;
public :
	inline ARE::utils::RecursiveMutex & FTBMutex(void) { return _FTBMutex ; }

protected :
	std::string _DiskSpaceDirectory ;
public :
	inline const std::string & DiskSpaceDirectory(void) const { return _DiskSpaceDirectory ; }

protected :
	__int64 _nInputTableBlocksWaited ;
	__int64 _InputTableBlocksWaitPeriodTotal ;
	__int64 _InputTableGetTimeTotal ; // time of Functio::GetFTB(). in milliseconds.
	__int64 _FileLoadTimeTotal ; // in milliseconds.
	__int64 _FileSaveTimeTotal ; // in milliseconds.
	__int64 _FTBComputationTimeTotal ; // in milliseconds.
	__int64 _nTableBlocksLoaded ;
	__int64 _nTableBlocksSaved ;
	__int64 _CurrentDiskMemorySpaceCached ;
	__int64 _MaximumDiskMemorySpaceCached ;
	int _nDiskTableBlocksInMemory ;
	int _MaximumNumConcurrentDiskTableBlocksInMemory ;
	int _nFTBsLoadedPerBucket[MAX_NUM_BUCKETS] ;
	//
	int _InputFTBWait_BucketIDX[128] ;
	int _InputFTBWait_BlockIDX[128] ;
	//
public :
	void ResetStatistics(void)
	{ 
		_nInputTableBlocksWaited = _InputTableBlocksWaitPeriodTotal = _nTableBlocksLoaded = _nTableBlocksSaved = 0 ; 
		_InputTableGetTimeTotal = _FileLoadTimeTotal = _FileSaveTimeTotal = _FTBComputationTimeTotal = 0 ;
		_CurrentDiskMemorySpaceCached = _MaximumDiskMemorySpaceCached = 0 ;
		_nDiskTableBlocksInMemory = _MaximumNumConcurrentDiskTableBlocksInMemory = 0 ;
		for (int i = 0 ; i < MAX_NUM_BUCKETS ; i++) 
			_nFTBsLoadedPerBucket[i] = 0 ;
	}
	inline __int64 nInputTableBlocksWaited(void) const { return _nInputTableBlocksWaited ; }
	inline void InputTableBlockWaitDetails(int i, int & BucketIDX, __int64 & BlockIDX) const { BucketIDX = _InputFTBWait_BucketIDX[i] ; BlockIDX = _InputFTBWait_BlockIDX[i] ; }
	inline __int64 InputTableBlocksWaitPeriodTotal(void) const { return _InputTableBlocksWaitPeriodTotal ; }
	inline __int64 InputTableGetTimeTotal(void) const { return _InputTableGetTimeTotal ; }
	inline __int64 FileLoadTimeTotal(void) const { return _FileLoadTimeTotal ; }
	inline __int64 FileSaveTimeTotal(void) const { return _FileSaveTimeTotal ; }
	inline __int64 FTBComputationTimeTotal(void) const { return _FTBComputationTimeTotal ; }
	inline __int64 nTableBlocksLoaded(void) const { return _nTableBlocksLoaded ; }
	inline __int64 nTableBlocksSaved(void) const { return _nTableBlocksSaved; }
	inline int nFTBsLoadedPerBucket(int IDX) const { return _nFTBsLoadedPerBucket[IDX] ; }
	inline void NoteInputTableGetTime(DWORD t)
	{
		ARE::utils::AutoLock lock(_FTBMutex) ;
		_InputTableGetTimeTotal += t ;
	}
	inline void NoteFileLoadTime(DWORD t)
	{
		ARE::utils::AutoLock lock(_FTBMutex) ;
		_FileLoadTimeTotal += t ;
	}
	inline void NoteFileSaveTime(DWORD t)
	{
		ARE::utils::AutoLock lock(_FTBMutex) ;
		_FileSaveTimeTotal += t ;
	}
	inline void NoteFTBComputationTime(DWORD t)
	{
		ARE::utils::AutoLock lock(_FTBMutex) ;
		_FTBComputationTimeTotal += t ;
	}
	inline void NoteInputTableBlocksWait(int BucketIDX, __int64 BlockIDX, bool Increment, long WaitInMilliseconds)
	{
		ARE::utils::AutoLock lock(_FTBMutex) ;
		_InputTableBlocksWaitPeriodTotal += WaitInMilliseconds ;
		if (Increment) {
//if (_nInputTableBlocksWaited < 128) {
//_InputFTBWait_BucketIDX[_nInputTableBlocksWaited] = BucketIDX ;
//_InputFTBWait_BlockIDX[_nInputTableBlocksWaited] = BlockIDX ;
//}
			++_nInputTableBlocksWaited ;
			}
	}
	inline void IncrementnTableBlocksLoaded(int IDX)
	{
		ARE::utils::AutoLock lock(_FTBMutex) ;
		++_nTableBlocksLoaded ;
		if (IDX >= 0) 
			_nFTBsLoadedPerBucket[IDX]++ ;
	}
	inline void IncrementnTableBlocksSaved(void)
	{
		ARE::utils::AutoLock lock(_FTBMutex) ;
		++_nTableBlocksSaved ;
	}
	inline __int64 CurrentDiskMemorySpaceCached(void) const { return _CurrentDiskMemorySpaceCached ; }
	inline __int64 MaximumDiskMemorySpaceCached(void) const { return _MaximumDiskMemorySpaceCached ; }
	inline int nDiskTableBlocksInMemory(void) const { return _nDiskTableBlocksInMemory ; }
	inline int MaximumNumConcurrentDiskTableBlocksInMemory(void) const { return _MaximumNumConcurrentDiskTableBlocksInMemory ; }
	void NoteDiskMemoryBlockLoaded(__int64 Space) 
	{ 
		ARE::utils::AutoLock lock(_FTBMutex) ;
		++_nDiskTableBlocksInMemory ;
		if (_nDiskTableBlocksInMemory > _MaximumNumConcurrentDiskTableBlocksInMemory) 
			_MaximumNumConcurrentDiskTableBlocksInMemory = _nDiskTableBlocksInMemory ;
		_CurrentDiskMemorySpaceCached += Space ;
		if (_CurrentDiskMemorySpaceCached > _MaximumDiskMemorySpaceCached)
			_MaximumDiskMemorySpaceCached = _CurrentDiskMemorySpaceCached ;
	}
	void NoteDiskMemoryBlockUnLoaded(__int64 Space) 
	{
		ARE::utils::AutoLock lock(_FTBMutex) ;
		--_nDiskTableBlocksInMemory ;
		_CurrentDiskMemorySpaceCached -= Space ;
	}
	void LogStatistics(time_t ttStart, time_t ttFinish) ;

public :

	virtual int Initialize(ARE::ARP & Problem)
	{
		if (NULL != _Problem) {
			if (&Problem != _Problem) 
				// cannot initialize twice, one atop another; must do reset in between.
				return 1 ;
			}
		_Problem = &Problem ;
		return 0 ;
	}
	virtual int Destroy(void) ;
	Workspace(const char *BEEMDiskSpaceDirectory)
		:
		_IsValid(true), 
		_ProblemBelongsToWorkspace(false), 
		_Problem(NULL), 
		_HasFatalError(false), 
		_ExplanationList(NULL), 
		_nInputTableBlocksWaited(0), 
		_InputTableBlocksWaitPeriodTotal(0), 
		_InputTableGetTimeTotal(0), 
		_FileLoadTimeTotal(0), 
		_FileSaveTimeTotal(0), 
		_FTBComputationTimeTotal(0), 
		_nTableBlocksLoaded(0), 
		_nTableBlocksSaved(0), 
		_CurrentDiskMemorySpaceCached(0), 
		_MaximumDiskMemorySpaceCached(0), 
		_nDiskTableBlocksInMemory(0), 
		_MaximumNumConcurrentDiskTableBlocksInMemory(0)
	{
		if (NULL != BEEMDiskSpaceDirectory) 
			_DiskSpaceDirectory = BEEMDiskSpaceDirectory ;
	}
	virtual ~Workspace(void)
	{
		Destroy() ;
	}
} ;

} // namespace ARE

#endif // ARE_workspace_HXX_INCLUDED
