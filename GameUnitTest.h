#pragma once

#ifndef __GAME_UNIT_TEST_H__
#define __GAME_UNIT_TEST_H__

#include "ServerManage.h"
class CGameTestRecord
{
public:
    CGameTestRecord();
    ~CGameTestRecord();
    void CountTimes(const char* pName,int count);
    void Print();
public:
    __int64     m_nUserGameMoney;
	__int64     m_nUserStartGameMoney;
    int         m_nTestTimes;
    int         m_nBetScore;

    __int64     m_nBetAmount;
    __int64     m_nWinAmount;
	__int64     m_nFreeWinAmount;
	__int64     m_nMaliWinAmount;
	__int64		m_nTotalWinAmount;
	__int64     m_nJackpotWinAmount;
    __int64     m_nLeftGameScore;
    std::unordered_map<std::string,int>     m_Counter;
	int			m_ScoreRecord[30];
};

struct  cellcount
{
	cellcount(){
		score=0;
		count=0;
	}
	int64_t score;
	int count;
};
class CGameUnitTest : public CServerGameDesk
{
public:
    CGameUnitTest();
    virtual ~CGameUnitTest();
    virtual void  TestStart();
	virtual bool  OnTimer(UINT uTimerID);
protected:
	bool TestLoop(BYTE byDeskStation,int times);
    void TestNormal(BYTE byDeskStation,int bet);
	void TestFree(BYTE byDeskStation);
	void TestMali(BYTE byDeskStation);
	void TestNormals(BYTE byDeskStation,int bet);
	void TestFrees(BYTE byDeskStation);
	void TestMalis(BYTE byDeskStation);
    void TestBonus(BYTE byDeskStation);
protected:
    bool VTestLoop(BYTE byDeskStation,int times);
    void VTestNormals(BYTE byDeskStation,int bet);
    void VTestFrees(BYTE byDeskStation);
    void VTestMalis(BYTE byDeskStation);
    void VTestBonus(BYTE byDeskStation);
	bool TestPlayOnce();
	virtual bool OnCalculateHitLines(int payCoin,__int64& winScore,
		int* retCells,int retCellNum,int* retHitLines,int retHitLinesNum,
		int* retHitCellPos,int retHitCellPosNum,std::map<int,cellcount> &wincellKey,__int64 bet);
	int calcOneLine(int _lineIdx,int payCoin,__int64& winScore,
		int* retCells,int* retHitLines,
		int* retHitCellPos,std::map<int,cellcount> &wincellKey,__int64 bet);
	 int checkOneLine(const std::vector<int>& _line, int& count, int& wildCount,__int64 bet);

     bool TestMaliGame(BYTE byDeskStation,int minmul,int maxmul);
     bool TestNormalGame(BYTE byDeskStation,int minmul,int maxmul);
public:
	 std::map<int,cellcount> m_wincellKey;
	 std::map<int,cellcount> m_FreewincellKey;
protected:
	void CollectRecordData();
    void PrintRecord();
private:
    CGameUserInfo* m_pUserInfo;

    CGameTestRecord m_RecordData;
};

#endif