#include "stdafx.h"
#include "GameUnitTest.h"
#include "GameRoomConfig.h"
#include "GameUtils.h"
#include "UserGamePlayRecored.h"

#define UNIT_TEST_TIMES 1000
#define UNIT_TEST_MONEY 99999999999999
#define UNIT_TEST_BET   90
#define TIMER_PLAY_ONCE 1024
#define PLAYER_TIME_MIN 20
#define PLAYER_TIME_MAX 25
#define TEST_MODE       5           //1:单人模式 2:多人模式 3:虚拟模式 4:小玛利结果数据 5:普通数据
#define MIN_MUL         75           //生成数据最小倍数
#define MAX_MUL         1000         //生成数据最大倍数

CGameTestRecord::CGameTestRecord()
{
    m_nBetScore = UNIT_TEST_BET;
    m_nTestTimes = UNIT_TEST_TIMES;
    m_nUserGameMoney = UNIT_TEST_MONEY;
	m_nUserStartGameMoney=UNIT_TEST_MONEY;
    m_nBetAmount = 0;
    m_nWinAmount = 0;
	m_nFreeWinAmount = 0;
	m_nMaliWinAmount = 0;
	m_nJackpotWinAmount = 0;
	m_nTotalWinAmount = 0;
	memset(m_ScoreRecord,0,sizeof(m_ScoreRecord));
}

CGameTestRecord::~CGameTestRecord()
{

}

void CGameTestRecord::CountTimes(const char* pName,int count)
{
    auto p = m_Counter.find(pName);
    if (p!=m_Counter.end())
    {
        p->second += count;
    }
    else
    {
        m_Counter[pName] = count;
    }
}

void CGameTestRecord::Print()
{
    GAME_LOG_INFO_FMT("初始分数:%lld,测试次数:%d,单次押注:%d,",m_nUserStartGameMoney,m_nTestTimes,m_nBetScore);

    auto it = m_Counter.begin();
    while(it!=m_Counter.end())
    {
        GAME_LOG_INFO_FMT("%s:%d",it->first.c_str(),it->second);
        ++it;
    }
	GAME_LOG_INFO_FMT("输赢情况,赢分总数:%lld,剩余金币:%lld 普通赢分:%lld 免费赢分:%lld 小玛丽赢分:%lld 奖池赢分:%lld",m_nTotalWinAmount,m_nUserGameMoney,m_nWinAmount,m_nFreeWinAmount,m_nMaliWinAmount,m_nJackpotWinAmount);
    
	GAME_LOG_INFO_FMT("倍数情况,0倍:[%d] 0-0.5倍:[%d] 0.5-1倍:[%d] 1-3倍:[%d] 3-5倍:[%d] 5-7倍:[%d] 7-9倍:[%d] 9-11倍:[%d] 11-13倍:[%d] 13-15倍:[%d] 15-17倍:[%d] 17-19倍:[%d] 19-21倍:[%d] 21-25倍:[%d] 25-30倍:[%d] 30-35倍:[%d] 35-70倍:[%d] 70-100倍:[%d] 100-150倍:[%d] 150-200倍:%d 200-N倍:%d \
		",m_ScoreRecord[0],m_ScoreRecord[1],m_ScoreRecord[2],m_ScoreRecord[3],m_ScoreRecord[4],m_ScoreRecord[5],m_ScoreRecord[6],m_ScoreRecord[7],m_ScoreRecord[8],m_ScoreRecord[9],m_ScoreRecord[10],m_ScoreRecord[11],m_ScoreRecord[12],
		m_ScoreRecord[13],m_ScoreRecord[14],m_ScoreRecord[15],m_ScoreRecord[16],m_ScoreRecord[17],m_ScoreRecord[18],m_ScoreRecord[19],m_ScoreRecord[20]);
}

CGameUnitTest::CGameUnitTest()
{
    m_pUserInfo = nullptr;
	m_wincellKey.clear();
	m_FreewincellKey.clear();
}

CGameUnitTest::~CGameUnitTest()
{

}



void CGameUnitTest::TestStart()
{
	static bool on = true;
	if (TEST_MODE == 1)   //仅一张桌子一个机器人，跑N吃
	{
		if (on)
		{
			on = false;
			auto info = onDeskJoinRobot(m_RecordData.m_nUserGameMoney);
			if (info != nullptr)
			{
				m_pUserInfo = info;
                m_pUserInfo->m_UserData.isVirtual=0;
				TestLoop(info->m_UserData.bDeskStation,m_RecordData.m_nTestTimes);

				CollectRecordData();

				onDeskLeaveRobot(m_pUserInfo);

				PrintRecord();
			}
		}
	}
    else if (TEST_MODE==3)
    {
        if (on)
        {
            on = false;
            auto info = onDeskJoinRobot(m_RecordData.m_nUserGameMoney);
            if (info != nullptr)
            {
                m_pUserInfo = info;
                m_pUserInfo->m_UserData.isVirtual=0;
                VTestLoop(info->m_UserData.bDeskStation,m_RecordData.m_nTestTimes);

                CollectRecordData();

                onDeskLeaveRobot(m_pUserInfo);

                PrintRecord();
            }
        }
    }
    else if (TEST_MODE==4)
    {
        if (on)
        {
            on = false;
            auto info = onDeskJoinRobot(m_RecordData.m_nUserGameMoney);
            if (info != nullptr)
            {
                m_pUserInfo = info;
                m_pUserInfo->m_UserData.isVirtual=0;
                m_UserData[info->m_UserData.bDeskStation].m_cellScore=UNIT_TEST_BET;
                for (int i=0;i<UNIT_TEST_TIMES;)
                {
                    if(TestMaliGame(info->m_UserData.bDeskStation,MIN_MUL,MAX_MUL))
                    {
                        i++;
                    }
                }
            }
        }
    }
    else if (TEST_MODE==5)
    {
        if (on)
        {
            on = false;
            auto info = onDeskJoinRobot(m_RecordData.m_nUserGameMoney);
            if (info != nullptr)
            {
                m_pUserInfo = info;
                m_pUserInfo->m_UserData.isVirtual=0;
                m_UserData[info->m_UserData.bDeskStation].m_cellScore=UNIT_TEST_BET;
                for (int i=0;i<UNIT_TEST_TIMES;)
                {
                    if(TestNormalGame(info->m_UserData.bDeskStation,MIN_MUL,MAX_MUL))
                    {
                        i++;
                    }
                }
            }
        }
    }
	//每张桌子1个机器人，2-5秒随机时间，每个人跑N次
	else
	{
		auto info = onDeskJoinRobot(m_RecordData.m_nUserGameMoney);
		if (info != nullptr)
		{
			m_pUserInfo = info;
			int seconds = m_DeskRand.myi_rand(PLAYER_TIME_MIN,PLAYER_TIME_MAX);
			SetTimer(TIMER_PLAY_ONCE,seconds);
		}
	}
}

void CGameUnitTest::CollectRecordData()
{
	if (m_pUserInfo!=nullptr)
	{
		/*auto PlayRecord= m_pUserInfo->m_pGameUserPlayRecord->CurData();
		m_RecordData.m_nBetAmount =PlayRecord.BetAmount();
		m_RecordData.m_nWinAmount = PlayRecord.WinAmount();*/
		m_RecordData.m_nLeftGameScore = GetUserGameMoney(m_pUserInfo->m_UserData.bDeskStation);
	}
}

bool CGameUnitTest::OnTimer(UINT uTimerID)
{
	switch (uTimerID)
	{
	case TIMER_PLAY_ONCE:
		KillTimer(TIMER_PLAY_ONCE);
		TestPlayOnce();
		break;
	default:
		break;
	}
	return false;
}

bool CGameUnitTest::VTestLoop(BYTE byDeskStation,int times)
{
    auto& user = m_UserData[byDeskStation];

    for (int i=0;i<times;)
    {
        //m_pLogGen->StartLine();

        if (user.m_nextMode == GAME_MODE_GENERAL)
        {
            ++i;
            if (GetUserGameMoney(byDeskStation) < m_RecordData.m_nBetScore)
            {
                //m_pLogGen->AppendItem("Error","UserMoney is not enough,stop test,testtimes:%d",i+1);
                break;
            }

            VTestNormals(byDeskStation,m_RecordData.m_nBetScore);
            if (user.m_nextMode==GAME_MODE_FREE)
            {
                m_RecordData.CountTimes("触发免费",1);
            }
        }
        else if(user.m_nextMode == GAME_MODE_FREE)
        {
            VTestFrees(byDeskStation);
        }
        else if (user.m_nextMode==GAME_MODE_MALI)
        {
            VTestMalis(byDeskStation);
        }
        else
        {
            //LogError("Unknow game mode triggered:%d",user.m_nextMode);
        }


        //m_pLogGen->EndLine();
    }
    auto it=m_wincellKey.begin();

    while (it!=m_wincellKey.end())
    {
        GAME_LOG_INFO_FMT("图标情况,cell:%d  winscore:%lld count:%d",it->first,it->second.score,it->second.count);
        it++;
    }

    it=m_FreewincellKey.begin();

    while (it!=m_FreewincellKey.end())
    {
        GAME_LOG_INFO_FMT("免费图标情况,cell:%d  winscore:%lld count:%d",it->first,it->second.score,it->second.count);
        it++;
    }

    return true;
}

void CGameUnitTest::VTestNormals(BYTE byDeskStation,int bet)
{
    int64_t userScore=GetUserGameMoney(byDeskStation);
    m_RecordData.m_nUserGameMoney-=bet;
    m_RecordData.CountTimes("Roll",1);
    GameUserData& User = m_UserData[byDeskStation];
    User.m_currentMode = User.m_nextMode;
    User.m_cellScore=bet;
    __int64 winScore = 0;
    int hitCells[GAME_CELLS];
    bool isEnterFreeGame = false;
    bool isEnterPanGoldGame = false;
    int hitLines[GAME_LINE_COUNT];
    int maLiTimes = 0;
    int freeTimes = 0;
    bool jackpot = false;
    int64_t ws = 0;
    int winType = GAME_WINTYPE_NOEFFECT;
    auto m_pConfig = GetGradeGameConfig<GameRoomConfig>(User.m_cellScore,byDeskStation);

    for(int i = 0; i < 1000; ++i)
    {
        memset(hitCells,0,sizeof(int)*GAME_CELLS);
        memset(hitLines,0,sizeof(int)*GAME_LINE_COUNT);
        winScore = 0;
        m_pConfig->OnScrollCells(0,User.m_lastCells,GAME_CELLS);



        maLiTimes = m_pConfig->calculateMaliTimes(User.m_lastCells,GAME_CELLS);
        freeTimes = m_pConfig->calculateFreeTimes(User.m_lastCells,GAME_CELLS);
        jackpot = m_pConfig->isTriggerJackpotBouns(User.m_lastCells,GAME_CELLS);
        if (maLiTimes > 0 && freeTimes > 0)
        {
            continue;
        }
        
        m_pConfig->OnCalculateHitLines(User.m_cellScore,winScore,m_UserData[byDeskStation].m_lastCells,
            GAME_CELLS,hitLines,GAME_LINE_COUNT,hitCells,GAME_CELLS);
        winType = m_pConfig->calcWinTypeByWinscore(winScore,User.m_cellScore);

        break;
    }
    int64_t test=0;
    OnCalculateHitLines(User.m_cellScore,test,m_UserData[byDeskStation].m_lastCells,
        GAME_CELLS,hitLines,GAME_LINE_COUNT,hitCells,GAME_CELLS,m_wincellKey,User.m_cellScore);
    if(freeTimes > 0)
    {
        User.m_nFreeTimes += freeTimes;
        User.m_nextMode = GAME_MODE_FREE;
        User.m_nTotalFreeTimes += freeTimes;
    }

    if (maLiTimes > 0)
    {
        User.m_maliTimes += maLiTimes;
        User.m_nextMode = GAME_MODE_MALI;
        m_RecordData.CountTimes("Mali",1);
    }

    if (jackpot)
    {
        winType = GAME_WINTYPE_JACKPOT;
        //奖池列表
        std::unordered_map<std::string,__int64> um;
        QueryJackpotDataByBet(User.m_cellScore,um);
        ws=um.begin()->second;
        m_RecordData.m_nJackpotWinAmount+=ws;
        m_RecordData.CountTimes("奖池",1);
    }
    //AddRoundWinScore(byDeskStation,ws+winScore,User.m_nextMode==GAME_MODE_GENERAL?true:false);

    //切换模式
    if (User.m_maliTimes>0)
    {
        User.m_nextMode=GAME_MODE_MALI;
    }
    else if (User.m_nFreeTimes>0)
    {
        User.m_nextMode=GAME_MODE_FREE;
    }else{
        User.m_nextMode = GAME_MODE_GENERAL;
    }

    if (User.m_currentMode == GAME_MODE_FREE)
    {
        User.m_nTotalFreeModeWinScore += winScore;
        // 退出免费
        if (User.m_nFreeTimes <= 0 && User.m_nextMode != GAME_MODE_MALI)
        {
            User.m_nextMode = GAME_MODE_GENERAL;
        }
    }

    /*if(winScore > 0)
    {
    winScore = UserWinScore(byDeskStation,winScore,User.m_cellScore);
    }*/
    // LogWrite(byDeskStation);
    // 免费进小玛丽
    if (User.m_currentMode==GAME_MODE_FREE && User.m_nextMode == GAME_MODE_MALI)
    {
        User.m_bFreeToMali = true;
    }

    if (User.m_currentMode==GAME_MODE_FREE || User.m_nextMode==GAME_MODE_FREE)
    {
        if (User.m_nextMode != GAME_MODE_FREE && !User.m_bFreeToMali)
        {
            User.m_nTotalFreeModeWinScore = 0;
            User.m_nTotalFreeTimes = 0;
        }
    }

    User.m_lastWinScore = winScore;
    m_RecordData.m_nUserGameMoney+=winScore+ws;
    m_RecordData.m_nWinAmount+=winScore;
    m_RecordData.m_nTotalWinAmount+=winScore+ws;
    if (User.m_nextMode == GAME_MODE_MALI)
    {
        User.m_nTotalWinScore = 0;
        User.m_nTotalFreeModeWinScore = 0;
        User.m_nTotalFreeTimes = 0;
    }

    double winbet=(double)User.m_lastWinScore/m_RecordData.m_nBetScore;
    if (winbet<=0)
    {
        m_RecordData.m_ScoreRecord[0]++;
    }else if (winbet<0.5)
    {
        m_RecordData.m_ScoreRecord[1]++;
    }else if (winbet<1)
    {
        m_RecordData.m_ScoreRecord[2]++;
    }else if (winbet<3)
    {
        m_RecordData.m_ScoreRecord[3]++;
    }else if (winbet<5)
    {
        m_RecordData.m_ScoreRecord[4]++;
    }else if (winbet<7)
    {
        m_RecordData.m_ScoreRecord[5]++;
    }else if (winbet<9)
    {
        m_RecordData.m_ScoreRecord[6]++;
    }else if (winbet<11)
    {
        m_RecordData.m_ScoreRecord[7]++;
    }else if (winbet<13)
    {
        m_RecordData.m_ScoreRecord[8]++;
    }else if (winbet<15)
    {
        m_RecordData.m_ScoreRecord[9]++;
    }else if (winbet<17)
    {
        m_RecordData.m_ScoreRecord[10]++;
    }else if (winbet<19)
    {
        m_RecordData.m_ScoreRecord[11]++;
    }else if (winbet<21)
    {
        m_RecordData.m_ScoreRecord[12]++;
    }else if (winbet<25)
    {
        m_RecordData.m_ScoreRecord[13]++;
    }else if (winbet<30)
    {
        m_RecordData.m_ScoreRecord[14]++;
    }else if (winbet<35)
    {
        m_RecordData.m_ScoreRecord[15]++;
    }else if (winbet<70)
    {
        m_RecordData.m_ScoreRecord[16]++;
    }else if (winbet<100)
    {
        m_RecordData.m_ScoreRecord[17]++;
    }else if (winbet<150)
    {
        m_RecordData.m_ScoreRecord[18]++;
    }else if (winbet<200)
    {
        m_RecordData.m_ScoreRecord[19]++;
    }else{
        m_RecordData.m_ScoreRecord[20]++;
    }
}

void CGameUnitTest::VTestFrees(BYTE byDeskStation)
{
    m_RecordData.CountTimes("FreeRoll",1);
    GameUserData& User = m_UserData[byDeskStation];
    User.m_currentMode = User.m_nextMode;
    User.m_nFreeTimes -= 1;
    __int64 winScore = 0;
    int hitCells[GAME_CELLS];
    bool isEnterFreeGame = false;
    bool isEnterPanGoldGame = false;
    int hitLines[GAME_LINE_COUNT];
    int maLiTimes = 0;
    int freeTimes = 0;
    bool jackpot = false;
    int64_t ws = 0;
    int winType = GAME_WINTYPE_NOEFFECT;
    auto m_pConfig = GetGradeGameConfig<GameRoomConfig>(User.m_cellScore,byDeskStation);

    for(int i = 0; i < 1000; ++i)
    {
        memset(hitCells,0,sizeof(int)*GAME_CELLS);
        memset(hitLines,0,sizeof(int)*GAME_LINE_COUNT);
        winScore = 0;
        m_pConfig->OnScrollCells(3,User.m_lastCells,GAME_CELLS);



        maLiTimes = m_pConfig->calculateMaliTimes(User.m_lastCells,GAME_CELLS);
        freeTimes = m_pConfig->calculateFreeTimes(User.m_lastCells,GAME_CELLS);
        jackpot = m_pConfig->isTriggerJackpotBouns(User.m_lastCells,GAME_CELLS);
        if (maLiTimes > 0 && freeTimes > 0)
        {
            continue;
        }
        
        m_pConfig->OnCalculateHitLines(User.m_cellScore,winScore,m_UserData[byDeskStation].m_lastCells,
            GAME_CELLS,hitLines,GAME_LINE_COUNT,hitCells,GAME_CELLS);
        winType = m_pConfig->calcWinTypeByWinscore(winScore,User.m_cellScore);
        
        break;
    }
    int64_t test=0;
    OnCalculateHitLines(User.m_cellScore,test,m_UserData[byDeskStation].m_lastCells,
        GAME_CELLS,hitLines,GAME_LINE_COUNT,hitCells,GAME_CELLS,m_FreewincellKey,User.m_cellScore);
    if(freeTimes > 0)
    {
        User.m_nFreeTimes += freeTimes;
        User.m_nextMode = GAME_MODE_FREE;
        User.m_nTotalFreeTimes += freeTimes;
        m_RecordData.CountTimes("免费触发免费",1);
        m_RecordData.CountTimes("free_freeroll",freeTimes);
    }

    if (maLiTimes > 0)
    {
        User.m_maliTimes += maLiTimes;
        User.m_nextMode = GAME_MODE_MALI;
        m_RecordData.CountTimes("FreeMali",1);
    }

    if (jackpot)
    {
        winType = GAME_WINTYPE_JACKPOT;
        m_RecordData.CountTimes("免费奖池",1);
        //奖池列表
        std::unordered_map<std::string,__int64> um;
        QueryJackpotDataByBet(User.m_cellScore,um);
        ws=um.begin()->second;
        m_RecordData.m_nJackpotWinAmount+=ws;
    }
    //AddRoundWinScore(byDeskStation,ws+winScore,User.m_nextMode==GAME_MODE_GENERAL?true:false);
    /*if(winScore > 0)
    {
        winScore = UserWinScore(byDeskStation,winScore,User.m_cellScore);
    }*/

    if (User.m_currentMode == GAME_MODE_FREE)
    {
        User.m_nTotalFreeModeWinScore += winScore;
        // 退出免费
        if (User.m_nFreeTimes <= 0 && User.m_nextMode != GAME_MODE_MALI)
        {
            User.m_nextMode = GAME_MODE_GENERAL;
        }
    }

    // 免费进小玛丽
    if (User.m_currentMode==GAME_MODE_FREE && User.m_nextMode == GAME_MODE_MALI)
    {
        User.m_bFreeToMali = true;
    }

    if (User.m_currentMode==GAME_MODE_FREE || User.m_nextMode==GAME_MODE_FREE)
    {
        if (User.m_nextMode != GAME_MODE_FREE && !User.m_bFreeToMali)
        {
            User.m_nTotalFreeModeWinScore = 0;
            User.m_nTotalFreeTimes = 0;
        }
    }

    User.m_lastWinScore = winScore;
    m_RecordData.m_nFreeWinAmount+=winScore;
    m_RecordData.m_nUserGameMoney+=winScore+ws;
    if (winScore>0)
    {
        m_RecordData.CountTimes("免费得分",1);
    }else{
        m_RecordData.CountTimes("免费不得分",1);
    }
    //m_RecordData.m_nWinAmount+=winScore;
    m_RecordData.m_nTotalWinAmount+=winScore+ws;
    if (User.m_nextMode == GAME_MODE_MALI)
    {
        User.m_nTotalWinScore = 0;
    }
    //LogWrite(byDeskStation);
}
void CGameUnitTest::VTestMalis(BYTE byDeskStation){

    GameUserData& User = m_UserData[byDeskStation];
    User.m_currentMode = User.m_nextMode;
    auto m_pConfig = GetGradeGameConfig<GameRoomConfig>(User.m_cellScore,byDeskStation);
    if (User.m_nextMode == GAME_MODE_MALI && User.m_maliTimes > 0)
    {
        User.m_currentMode = User.m_nextMode;

        int idx = 0;
        int c4[GAME_MALI_CENTER_ICON_COUNT] = {0};
        int hitC4[GAME_MALI_CENTER_ICON_COUNT] = {0};
        uint64_t winscore = 0;
        int idxType = 0;
        m_pConfig->maliGameStart(User.m_maliCount,idx,c4,GAME_MALI_CENTER_ICON_COUNT,User.m_cellScore);
        m_pConfig->calcMaliResult(idx,c4,hitC4,GAME_MALI_CENTER_ICON_COUNT,User.m_cellScore,winscore,idxType);

        int winType = m_pConfig->calcWinTypeByWinscore(winscore,User.m_cellScore);
        if (idxType == GAME_MALI_EXIT)
        {
            User.m_maliTimes -= 1;
            User.m_maliCount = 0;
            if(User.m_maliTimes <= 0)
            {
                //如果是免费游戏进来的小玛丽，小玛丽结束后，继续进入免费模式
                if (User.m_bFreeToMali)   
                {
                    User.m_nextMode = GAME_MODE_FREE;
                }
                else
                {
                    User.m_nextMode = GAME_MODE_GENERAL;
                }
            }
        }else{
            User.m_maliCount++;
        }

        m_pConfig->calcMaliHitCell(c4,GAME_MALI_CENTER_ICON_COUNT,hitC4,GAME_MALI_CENTER_ICON_COUNT,idxType);

        User.m_nTotalWinScore += winscore;
        m_RecordData.m_nMaliWinAmount+=winscore;
        m_RecordData.m_nUserGameMoney+=winscore;
        //m_RecordData.m_nWinAmount+=winscore;
        m_RecordData.m_nTotalWinAmount+=winscore;
        if (User.m_bFreeToMali)
        {
            User.m_nTotalFreeModeWinScore += winscore;
        }
       /* if (winscore>0)
        {
            winscore = UserWinScore(byDeskStation,winscore,User.m_cellScore);
        }*/
        //AddRoundWinScore(byDeskStation,winscore,User.m_nextMode==GAME_MODE_GENERAL?true:false);
        User.m_LastMaliIndex=idx;
        memcpy(User.m_CenterMaliIndex,c4,sizeof(User.m_CenterMaliIndex));
        if (User.m_maliTimes <= 0 && User.m_bFreeToMali && User.m_nFreeTimes == 0)
        {
            User.m_nextMode = GAME_MODE_GENERAL;
            User.m_nTotalFreeModeWinScore = 0;
            User.m_nTotalFreeTimes = 0;
            User.m_bFreeToMali = false;
        }
        // LogWrite(byDeskStation);
    }
}

bool CGameUnitTest::TestLoop(BYTE byDeskStation,int times)
{
    auto& user = m_UserData[byDeskStation];

    for (int i=0;i<times;)
    {
        //m_pLogGen->StartLine();

        if (user.m_nextMode == GAME_MODE_GENERAL)
        {
			++i;
            if (GetUserGameMoney(byDeskStation) < m_RecordData.m_nBetScore)
            {
                //m_pLogGen->AppendItem("Error","UserMoney is not enough,stop test,testtimes:%d",i+1);
                break;
            }
            
            TestNormals(byDeskStation,m_RecordData.m_nBetScore);
			if (user.m_nextMode==GAME_MODE_FREE)
			{
				m_RecordData.CountTimes("触发免费",1);
			}
        }
        else if(user.m_nextMode == GAME_MODE_FREE)
        {
            TestFrees(byDeskStation);
        }
		else if (user.m_nextMode==GAME_MODE_MALI)
		{
			TestMalis(byDeskStation);
		}
        else
        {
            //LogError("Unknow game mode triggered:%d",user.m_nextMode);
        }

		
        //m_pLogGen->EndLine();
    }
	auto it=m_wincellKey.begin();

	while (it!=m_wincellKey.end())
	{
		GAME_LOG_INFO_FMT("图标情况,cell:%d  winscore:%lld count:%d",it->first,it->second.score,it->second.count);
		it++;
	}

	it=m_FreewincellKey.begin();

	while (it!=m_FreewincellKey.end())
	{
		GAME_LOG_INFO_FMT("免费图标情况,cell:%d  winscore:%lld count:%d",it->first,it->second.score,it->second.count);
		it++;
	}
    
    return true;
}

void CGameUnitTest::TestNormal(BYTE byDeskStation,int bet)
{
	auto& user = m_UserData[byDeskStation];
	int64_t userScore=GetUserGameMoney(byDeskStation);
    OnHandleStartRoll(byDeskStation,bet);
    m_RecordData.CountTimes("Roll",1);
    auto m_pConfig = GetGradeGameConfig<GameRoomConfig>(bet,byDeskStation);
	/*if ((GetUserGameMoney(byDeskStation)-userScore+bet)/bet>=5)
	{
		 m_RecordData.CountTimes("5倍以上",1);
	}*/
	if (m_pConfig->isTriggerJackpotBouns(user.m_lastCells,GAME_CELLS))
	{
		int64_t winscore=GetUserGameMoney(byDeskStation)-userScore+bet-user.m_lastWinScore;
		m_RecordData.m_nJackpotWinAmount+=winscore;
		m_RecordData.CountTimes("奖池",1);
	}
	if (userScore-bet<GetUserGameMoney(byDeskStation))
	{
		m_RecordData.CountTimes("有得分",1);
	}
}

void CGameUnitTest::TestFree(BYTE byDeskStation)
{
	auto& user = m_UserData[byDeskStation];
	int64_t userScore=GetUserGameMoney(byDeskStation);
	OnHandleStartRoll(byDeskStation,user.m_cellScore);
	//m_RecordData.CountTimes("FreeRoll",1);
	m_RecordData.m_nFreeWinAmount+=user.m_lastWinScore;
    auto m_pConfig = GetGradeGameConfig<GameRoomConfig>(user.m_cellScore,byDeskStation);
	if (m_pConfig->isTriggerJackpotBouns(user.m_lastCells,GAME_CELLS))
	{
		int64_t winscore=GetUserGameMoney(byDeskStation)-userScore-user.m_lastWinScore;
		m_RecordData.m_nJackpotWinAmount+=winscore;
		m_RecordData.CountTimes("奖池",1);
	}
	/*if ((GetUserGameMoney(byDeskStation)-userScore)/user.m_cellScore>=5)
	{
		m_RecordData.CountTimes("5倍以上",1);
	}*/
}

void CGameUnitTest::TestMali(BYTE byDeskStation){
	auto& user = m_UserData[byDeskStation];
	int64_t userScore=GetUserGameMoney(byDeskStation);
	OnRequestMaliGame(byDeskStation);
	m_RecordData.m_nMaliWinAmount+=GetUserGameMoney(byDeskStation)-userScore;
	m_RecordData.CountTimes("Mali",1);
}

void CGameUnitTest::TestNormals(BYTE byDeskStation,int bet)
{
	int64_t userScore=GetUserGameMoney(byDeskStation);
	m_RecordData.m_nUserGameMoney-=bet;
    m_RecordData.CountTimes("Roll",1);
	GameUserData& User = m_UserData[byDeskStation];
	User.m_currentMode = User.m_nextMode;
	User.m_cellScore=bet;
	__int64 winScore = 0;
	int hitCells[GAME_CELLS];
	bool isEnterFreeGame = false;
	bool isEnterPanGoldGame = false;
	int hitLines[GAME_LINE_COUNT];
	int maLiTimes = 0;
	int freeTimes = 0;
	bool jackpot = false;
	int64_t ws = 0;
	int winType = GAME_WINTYPE_NOEFFECT;
    auto m_pConfig = GetGradeGameConfig<GameRoomConfig>(User.m_cellScore,byDeskStation);
    int maxmul=0;
    bool Couldfree=false;
    bool CouldSpecialGame=false;
    bool bControl=false;

    bControl = CheckControlParams(User.m_cellScore,m_pUserInfo[byDeskStation].m_UserData.userBuff,Couldfree,CouldSpecialGame,maxmul);

    if(false == UserLoseScore(byDeskStation,User.m_cellScore)){
        return;
    }
	for(int i = 0; i < 1000; ++i)
	{
		memset(hitCells,0,sizeof(int)*GAME_CELLS);
		memset(hitLines,0,sizeof(int)*GAME_LINE_COUNT);
		winScore = 0;
		m_pConfig->OnScrollCells(0,User.m_lastCells,GAME_CELLS);

		

		maLiTimes = m_pConfig->calculateMaliTimes(User.m_lastCells,GAME_CELLS);
		freeTimes = m_pConfig->calculateFreeTimes(User.m_lastCells,GAME_CELLS);
		jackpot = m_pConfig->isTriggerJackpotBouns(User.m_lastCells,GAME_CELLS);
		if (maLiTimes > 0 && freeTimes > 0)
		{
			continue;
		}
        //检查 是否可以进入免费
        if(freeTimes > 0 &&!Couldfree)
            continue;
        //检查 是否可以进入小玛丽
        if(maLiTimes > 0 &&!CouldSpecialGame)
            continue;
        //检查 是否可以进奖池
        if(jackpot && !UserCouldWinJackpot(byDeskStation,User.m_cellScore,"jackpot"))
            continue;
        m_pConfig->OnCalculateHitLines(User.m_cellScore,winScore,m_UserData[byDeskStation].m_lastCells,
            GAME_CELLS,hitLines,GAME_LINE_COUNT,hitCells,GAME_CELLS);
        winType = m_pConfig->calcWinTypeByWinscore(winScore,User.m_cellScore);
        if (bControl&&maxmul<(winScore*1.0f/User.m_cellScore))
        {
            continue;
        }
        break;
    }
	int64_t test=0;
	OnCalculateHitLines(User.m_cellScore,test,m_UserData[byDeskStation].m_lastCells,
		GAME_CELLS,hitLines,GAME_LINE_COUNT,hitCells,GAME_CELLS,m_wincellKey,User.m_cellScore);
	if(freeTimes > 0)
	{
		User.m_nFreeTimes += freeTimes;
		User.m_nextMode = GAME_MODE_FREE;
		User.m_nTotalFreeTimes += freeTimes;
	}

	if (maLiTimes > 0)
	{
		User.m_maliTimes += maLiTimes;
		User.m_nextMode = GAME_MODE_MALI;
		m_RecordData.CountTimes("Mali",1);
	}

	if (jackpot)
	{
		winType = GAME_WINTYPE_JACKPOT;
        WinJackpot(byDeskStation,ws,User.m_cellScore,"jackpot");
        m_RecordData.m_nJackpotWinAmount+=ws;
		m_RecordData.CountTimes("奖池",1);
	}
    //AddRoundWinScore(byDeskStation,ws+winScore,User.m_nextMode==GAME_MODE_GENERAL?true:false);

    //切换模式
    if (User.m_maliTimes>0)
    {
        User.m_nextMode=GAME_MODE_MALI;
    }
    else if (User.m_nFreeTimes>0)
    {
        User.m_nextMode=GAME_MODE_FREE;
    }else{
        User.m_nextMode = GAME_MODE_GENERAL;
    }

	if (User.m_currentMode == GAME_MODE_FREE)
	{
		User.m_nTotalFreeModeWinScore += winScore;
		// 退出免费
		if (User.m_nFreeTimes <= 0 && User.m_nextMode != GAME_MODE_MALI)
		{
			User.m_nextMode = GAME_MODE_GENERAL;
		}
	}

    if(winScore > 0)
    {
        winScore = UserWinScore(byDeskStation,winScore,User.m_cellScore);
    }
   // LogWrite(byDeskStation);
		// 免费进小玛丽
		if (User.m_currentMode==GAME_MODE_FREE && User.m_nextMode == GAME_MODE_MALI)
		{
			User.m_bFreeToMali = true;
		}

		if (User.m_currentMode==GAME_MODE_FREE || User.m_nextMode==GAME_MODE_FREE)
		{
			if (User.m_nextMode != GAME_MODE_FREE && !User.m_bFreeToMali)
			{
				User.m_nTotalFreeModeWinScore = 0;
				User.m_nTotalFreeTimes = 0;
			}
		}

		User.m_lastWinScore = winScore;
		m_RecordData.m_nUserGameMoney+=winScore+ws;
		m_RecordData.m_nWinAmount+=winScore;
		m_RecordData.m_nTotalWinAmount+=winScore+ws;
		if (User.m_nextMode == GAME_MODE_MALI)
		{
			User.m_nTotalWinScore = 0;
            User.m_nTotalFreeModeWinScore = 0;
            User.m_nTotalFreeTimes = 0;
		}

		double winbet=(double)User.m_lastWinScore/m_RecordData.m_nBetScore;
		if (winbet<=0)
		{
			m_RecordData.m_ScoreRecord[0]++;
		}else if (winbet<0.5)
		{
			m_RecordData.m_ScoreRecord[1]++;
		}else if (winbet<1)
		{
			m_RecordData.m_ScoreRecord[2]++;
		}else if (winbet<3)
		{
			m_RecordData.m_ScoreRecord[3]++;
		}else if (winbet<5)
		{
			m_RecordData.m_ScoreRecord[4]++;
		}else if (winbet<7)
		{
			m_RecordData.m_ScoreRecord[5]++;
		}else if (winbet<9)
		{
			m_RecordData.m_ScoreRecord[6]++;
		}else if (winbet<11)
		{
			m_RecordData.m_ScoreRecord[7]++;
		}else if (winbet<13)
		{
			m_RecordData.m_ScoreRecord[8]++;
		}else if (winbet<15)
		{
			m_RecordData.m_ScoreRecord[9]++;
		}else if (winbet<17)
		{
			m_RecordData.m_ScoreRecord[10]++;
		}else if (winbet<19)
		{
			m_RecordData.m_ScoreRecord[11]++;
		}else if (winbet<21)
		{
			m_RecordData.m_ScoreRecord[12]++;
		}else if (winbet<25)
		{
			m_RecordData.m_ScoreRecord[13]++;
		}else if (winbet<30)
		{
			m_RecordData.m_ScoreRecord[14]++;
		}else if (winbet<35)
		{
			m_RecordData.m_ScoreRecord[15]++;
		}else if (winbet<70)
		{
			m_RecordData.m_ScoreRecord[16]++;
		}else if (winbet<100)
		{
			m_RecordData.m_ScoreRecord[17]++;
		}else if (winbet<150)
		{
			m_RecordData.m_ScoreRecord[18]++;
		}else if (winbet<200)
		{
			m_RecordData.m_ScoreRecord[19]++;
		}else{
			m_RecordData.m_ScoreRecord[20]++;
		}
}

void CGameUnitTest::TestFrees(BYTE byDeskStation)
{
	 m_RecordData.CountTimes("FreeRoll",1);
	GameUserData& User = m_UserData[byDeskStation];
	User.m_currentMode = User.m_nextMode;
	 User.m_nFreeTimes -= 1;
	__int64 winScore = 0;
	int hitCells[GAME_CELLS];
	bool isEnterFreeGame = false;
	bool isEnterPanGoldGame = false;
	int hitLines[GAME_LINE_COUNT];
	int maLiTimes = 0;
	int freeTimes = 0;
	bool jackpot = false;
	int64_t ws = 0;
	int winType = GAME_WINTYPE_NOEFFECT;
    auto m_pConfig = GetGradeGameConfig<GameRoomConfig>(User.m_cellScore,byDeskStation);

    int maxmul=0;
    bool Couldfree=false;
    bool CouldSpecialGame=false;
    bool bControl=false;

    bControl = CheckControlParams(User.m_cellScore,m_pUserInfo[byDeskStation].m_UserData.userBuff,Couldfree,CouldSpecialGame,maxmul);
	for(int i = 0; i < 1000; ++i)
	{
		memset(hitCells,0,sizeof(int)*GAME_CELLS);
		memset(hitLines,0,sizeof(int)*GAME_LINE_COUNT);
		winScore = 0;
		m_pConfig->OnScrollCells(3,User.m_lastCells,GAME_CELLS);

		

		maLiTimes = m_pConfig->calculateMaliTimes(User.m_lastCells,GAME_CELLS);
		freeTimes = m_pConfig->calculateFreeTimes(User.m_lastCells,GAME_CELLS);
		jackpot = m_pConfig->isTriggerJackpotBouns(User.m_lastCells,GAME_CELLS);
		if (maLiTimes > 0 && freeTimes > 0)
		{
			continue;
		}
        //检查 是否可以进入免费
        if(freeTimes > 0 &&!Couldfree)
            continue;
        //检查 是否可以进入小玛丽
        if(maLiTimes > 0 &&!CouldSpecialGame)
            continue;
        //检查 是否可以进奖池
        if(jackpot && !UserCouldWinJackpot(byDeskStation,User.m_cellScore,"jackpot"))
            continue;
        m_pConfig->OnCalculateHitLines(User.m_cellScore,winScore,m_UserData[byDeskStation].m_lastCells,
            GAME_CELLS,hitLines,GAME_LINE_COUNT,hitCells,GAME_CELLS);
        winType = m_pConfig->calcWinTypeByWinscore(winScore,User.m_cellScore);
        if (bControl&&maxmul<(winScore*1.0f/User.m_cellScore))
        {
            continue;
        }
		break;
	}
	int64_t test=0;
	OnCalculateHitLines(User.m_cellScore,test,m_UserData[byDeskStation].m_lastCells,
		GAME_CELLS,hitLines,GAME_LINE_COUNT,hitCells,GAME_CELLS,m_FreewincellKey,User.m_cellScore);
	if(freeTimes > 0)
	{
		User.m_nFreeTimes += freeTimes;
		User.m_nextMode = GAME_MODE_FREE;
		User.m_nTotalFreeTimes += freeTimes;
		m_RecordData.CountTimes("免费触发免费",1);
		m_RecordData.CountTimes("free_freeroll",freeTimes);
	}

	if (maLiTimes > 0)
	{
		User.m_maliTimes += maLiTimes;
		User.m_nextMode = GAME_MODE_MALI;
		m_RecordData.CountTimes("FreeMali",1);
	}

	if (jackpot)
	{
		winType = GAME_WINTYPE_JACKPOT;
		m_RecordData.CountTimes("免费奖池",1);
        WinJackpot(byDeskStation,ws,User.m_cellScore,"jackpot");
        m_RecordData.m_nJackpotWinAmount+=ws;
	}
    //AddRoundWinScore(byDeskStation,ws+winScore,User.m_nextMode==GAME_MODE_GENERAL?true:false);
    if(winScore > 0)
    {
        winScore = UserWinScore(byDeskStation,winScore,User.m_cellScore);
    }

	if (User.m_currentMode == GAME_MODE_FREE)
	{
		User.m_nTotalFreeModeWinScore += winScore;
		// 退出免费
		if (User.m_nFreeTimes <= 0 && User.m_nextMode != GAME_MODE_MALI)
		{
			User.m_nextMode = GAME_MODE_GENERAL;
		}
	}

	// 免费进小玛丽
	if (User.m_currentMode==GAME_MODE_FREE && User.m_nextMode == GAME_MODE_MALI)
	{
		User.m_bFreeToMali = true;
	}

	if (User.m_currentMode==GAME_MODE_FREE || User.m_nextMode==GAME_MODE_FREE)
	{
		if (User.m_nextMode != GAME_MODE_FREE && !User.m_bFreeToMali)
		{
			User.m_nTotalFreeModeWinScore = 0;
			User.m_nTotalFreeTimes = 0;
		}
	}

	User.m_lastWinScore = winScore;
	m_RecordData.m_nFreeWinAmount+=winScore;
	m_RecordData.m_nUserGameMoney+=winScore+ws;
	if (winScore>0)
	{
		m_RecordData.CountTimes("免费得分",1);
	}else{
		m_RecordData.CountTimes("免费不得分",1);
	}
	//m_RecordData.m_nWinAmount+=winScore;
	m_RecordData.m_nTotalWinAmount+=winScore+ws;
	if (User.m_nextMode == GAME_MODE_MALI)
	{
		User.m_nTotalWinScore = 0;
	}
    //LogWrite(byDeskStation);
}
void CGameUnitTest::TestMalis(BYTE byDeskStation){
	
	GameUserData& User = m_UserData[byDeskStation];
	User.m_currentMode = User.m_nextMode;
    auto m_pConfig = GetGradeGameConfig<GameRoomConfig>(User.m_cellScore,byDeskStation);
    if (User.m_nextMode == GAME_MODE_MALI && User.m_maliTimes > 0)
    {
        User.m_currentMode = User.m_nextMode;

        int idx = 0;
        int c4[GAME_MALI_CENTER_ICON_COUNT] = {0};
		int hitC4[GAME_MALI_CENTER_ICON_COUNT] = {0};
        uint64_t winscore = 0;
        int idxType = 0;
        m_pConfig->maliGameStart(User.m_maliCount,idx,c4,GAME_MALI_CENTER_ICON_COUNT,User.m_cellScore);
        m_pConfig->calcMaliResult(idx,c4,hitC4,GAME_MALI_CENTER_ICON_COUNT,User.m_cellScore,winscore,idxType);

        int winType = m_pConfig->calcWinTypeByWinscore(winscore,User.m_cellScore);
        if (idxType == GAME_MALI_EXIT)
        {
            User.m_maliTimes -= 1;
			User.m_maliCount = 0;
            if(User.m_maliTimes <= 0)
            {
                //如果是免费游戏进来的小玛丽，小玛丽结束后，继续进入免费模式
                if (User.m_bFreeToMali)   
                {
                    User.m_nextMode = GAME_MODE_FREE;
                }
                else
                {
                    User.m_nextMode = GAME_MODE_GENERAL;
                }
            }
        }else{
			User.m_maliCount++;
		}

        m_pConfig->calcMaliHitCell(c4,GAME_MALI_CENTER_ICON_COUNT,hitC4,GAME_MALI_CENTER_ICON_COUNT,idxType);

        User.m_nTotalWinScore += winscore;
		m_RecordData.m_nMaliWinAmount+=winscore;
		m_RecordData.m_nUserGameMoney+=winscore;
		//m_RecordData.m_nWinAmount+=winscore;
		m_RecordData.m_nTotalWinAmount+=winscore;
        if (User.m_bFreeToMali)
        {
            User.m_nTotalFreeModeWinScore += winscore;
        }
        if (winscore>0)
        {
            winscore = UserWinScore(byDeskStation,winscore,User.m_cellScore);
        }
        //AddRoundWinScore(byDeskStation,winscore,User.m_nextMode==GAME_MODE_GENERAL?true:false);
		User.m_LastMaliIndex=idx;
		memcpy(User.m_CenterMaliIndex,c4,sizeof(User.m_CenterMaliIndex));
        if (User.m_maliTimes <= 0 && User.m_bFreeToMali && User.m_nFreeTimes == 0)
        {
            User.m_nextMode = GAME_MODE_GENERAL;
            User.m_nTotalFreeModeWinScore = 0;
            User.m_nTotalFreeTimes = 0;
            User.m_bFreeToMali = false;
        }
       // LogWrite(byDeskStation);
    }
}

bool CGameUnitTest::TestPlayOnce()
{
	static int allTimes = -1;
	if(allTimes == -1)
	{
		allTimes = m_RecordData.m_nTestTimes;
	}
	if (m_pUserInfo!=nullptr)
	{
		bool result = true;
		if(allTimes > 0)
		{
			allTimes -= 1;

			result = TestLoop(m_pUserInfo->m_UserData.bDeskStation,1);
		}
		else
		{
			result = false;
		}


		if(!result)
		{
			
			GAME_LOG_INFO_FMT("-----------------------机器人%d号完成测试，一下是测试结果:--------------------",m_pUserInfo->m_UserData.bDeskNO);

			CollectRecordData();

			onDeskLeaveRobot(m_pUserInfo);

			PrintRecord();
		}
		else
		{
			int seconds = m_DeskRand.myi_rand(PLAYER_TIME_MIN,PLAYER_TIME_MAX);
			SetTimer(TIMER_PLAY_ONCE,seconds);
		}
	}
	return false;
}

void CGameUnitTest::PrintRecord()
{
	m_RecordData.Print();
}

int CGameUnitTest::calcOneLine(int _lineIdx,
	int payCoin,
	__int64& winScore, 
	int* retCells,
	int* retHitLines, 
	int* retHitCellPos,
	std::map<int,cellcount> &wincellKey,__int64 bet)
{
    auto m_pConfig = DefaultGradeGameConfig<GameRoomConfig>(bet);
	if (_lineIdx >= 0 && _lineIdx < m_pConfig->m_cfgLines)
	{
		LineDataEx* pLine = &(m_pConfig->m_cfgArrayLine[_lineIdx]);
		if (pLine!=nullptr)
		{
			std::vector<int> l;
			std::vector<int> p;
			for (int i=0;i<pLine->m_MaxCount;++i)
			{
				int& v = pLine->m_Pos[i];
				l.push_back(retCells[v]);
				p.push_back(v);
			}

			int count = 0;
			int wildCount = 0;
			int cellType = checkOneLine(l,count,wildCount,bet);

			__int64 win = 0;
			// 计算得分，返回这条线是否可连
			auto calc = [m_pConfig,&retHitCellPos,payCoin,this,_lineIdx,&wincellKey](const std::vector<int>& _l,const std::vector<int>& _p,int _type,int _count,int _wildCount,__int64& _winscore)->bool
			{
				int  cellKey = _type*1000+_count;
				auto fitb = m_pConfig->m_cfgPaytable.find(cellKey);
				__int64 score = 0;
				if(fitb != m_pConfig->m_cfgPaytable.end())
				{
					score = payCoin/m_pConfig->m_cfgLines;
					score = score * fitb->second;
					wincellKey[cellKey].count++;
					wincellKey[cellKey].score+=score;
					_winscore = _winscore + score;
					std::vector<int> lineCells;
					for (int i=0;i<_count;++i)
					{
						retHitCellPos[_p[i]] = 1;
						lineCells.push_back(_l[i]);
					}

					__int64 extraWinScore = 0;

					//m_pConfig->OnExtraCalculateHitLine(payCoin/m_cfgLines,score,extraWinScore,lineCells,_lineIdx);
					_winscore = _winscore + extraWinScore;
					//m_wincellKey[1]+=extraWinScore;
					return true;
				}   
				return false;
			};
			bool result1 = calc(l,p,cellType,count,wildCount,win); //正线
			winScore += win;

			int linedata = 0;
			if (result1)
			{
				linedata = count;
			}

			if (count == GAME_CELL_COLUMES)
			{
				retHitLines[_lineIdx] = count;
				return wildCount;
			}

			retHitLines[_lineIdx] = linedata;
			return wildCount;
		}   
	}
	return -1;
}

bool CGameUnitTest::OnCalculateHitLines(int payCoin,__int64& winScore, int* retCells,int retCellNum,int* retHitLines,int retHitLinesNum, int* retHitCellPos,int retHitCellPosNum,std::map<int,cellcount> &wincellKey,__int64 bet)
{
    auto m_pConfig = DefaultGradeGameConfig<GameRoomConfig>(bet);
	if(retCellNum != m_pConfig->m_cfgCellNumber || retHitCellPosNum != m_pConfig->m_cfgCellNumber)
	{
		return false;
	}
	if(retHitLinesNum != m_pConfig->m_cfgLines)
	{
		return false;
	}
	__int64 win = 0;
	for (int i=0;i<m_pConfig->m_cfgLines;++i)
	{
		LineDataEx& _line = m_pConfig->m_cfgArrayLine[i];
		calcOneLine(i,payCoin,win,retCells,retHitLines,retHitCellPos,wincellKey,bet);
		winScore += win;
		win = 0;
	}
	return winScore > 0;
}

int CGameUnitTest::checkOneLine(const std::vector<int>& _line, int& count, int& wildCount,__int64 bet)
{
	if (_line.size() == 0)
	{
		return -1;
	}

	auto it = _line.begin();
	int _type = *it;
	count = 0;
	wildCount = 0;
	// 如果第一个图标是wild,这查找第一个不是wild的图标的类型

	do 
	{
		if (*it != GAME_CELL_WILD)
		{
			_type = *it;
			break;
		}
		else
		{
			++wildCount;
		}
		++it;
	} while (it != _line.end());

	//如果一行全部是wild
	if (wildCount == GAME_CELL_COLUMES)
	{
		count = wildCount;
		return _type;
	}

	it = _line.begin();

    auto m_pConfig = DefaultGradeGameConfig<GameRoomConfig>(bet);
	count = 0;
	wildCount = 0;
	do
	{
		const int& t = *it;
		if (t == _type)
		{
			++count;
		}
		else
		{
			if (t == GAME_CELL_WILD)
			{
				auto _it = m_pConfig->m_cfgWildNoMatchCells.begin();
				bool matched = false;
				while (_it != m_pConfig->m_cfgWildNoMatchCells.end())
				{
					if (_type == *_it)
					{
						matched = true;
						break;
					}
					++_it;
				}
				if (matched)
				{
					break;
				}
				++count;
				++wildCount;
			}
			else
			{
				break;
			}
		}
		++it;
	} while (it != _line.end());
	return _type;
}

bool CGameUnitTest::TestMaliGame(BYTE byDeskStation,int minmul,int maxmul)
{
    GameUserData& User = m_UserData[byDeskStation];
    auto m_pConfig = GetGradeGameConfig<GameRoomConfig>(User.m_cellScore,byDeskStation);
    int cells[GAME_CELLS]={0};
    while (true)
    {
        m_pConfig->OnScrollCells(0,User.m_lastCells,GAME_CELLS);
        if (m_pConfig->calculateMaliTimes(User.m_lastCells,GAME_CELLS))
        {
            break;
        }
    }
    int hitCells[GAME_CELLS];
    int hitLines[GAME_LINE_COUNT];
    int64_t winScore=0;
    int malitimes=m_pConfig->calculateMaliTimes(User.m_lastCells,GAME_CELLS);
    m_pConfig->OnCalculateHitLines(User.m_cellScore,winScore,m_UserData[byDeskStation].m_lastCells,
        GAME_CELLS,hitLines,GAME_LINE_COUNT,hitCells,GAME_CELLS);
    struct maliresult
    {
        maliresult()
        {
            idx=0;
            memset(c4,0,sizeof(c4));
        }
        int idx;
        int c4[GAME_MALI_CENTER_ICON_COUNT];
    };
    if (malitimes>15)
    {
        return false;
    }
    std::vector<maliresult> m_record;
    while (malitimes>0)
    {
        int idx = 0;
        int c4[GAME_MALI_CENTER_ICON_COUNT] = {0};
        int hitC4[GAME_MALI_CENTER_ICON_COUNT] = {0};
        uint64_t winscore = 0;
        int idxType = 0;
        m_pConfig->maliGameStart(User.m_maliCount,idx,c4,GAME_MALI_CENTER_ICON_COUNT,User.m_cellScore);
        m_pConfig->calcMaliResult(idx,c4,hitC4,GAME_MALI_CENTER_ICON_COUNT,User.m_cellScore,winscore,idxType);
        winScore+=winscore;
        maliresult result;
        result.idx=idx;
        memcpy(result.c4,c4,sizeof(c4));
        m_record.push_back(result);
        if (idxType == GAME_MALI_EXIT)
        {
            malitimes--;
            User.m_maliCount=0;
        }
        else
        {
            User.m_maliCount++;
        }
    }
    int mul=winScore*GAME_LINE/User.m_cellScore;
    if (mul>minmul&&mul<=maxmul)
    {
        CString str;
        auto it=m_record.begin();
        while(it!=m_record.end())
        {
            CString temp;
            temp.Format(_T("%d,%d,%d,%d,%d  "),it->idx,it->c4[0],it->c4[1],it->c4[2],it->c4[3]);
            str+=temp;
            it++;
        }
        GAME_LOG_INFO_FMT("%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d   %s    %d",User.m_lastCells[0],User.m_lastCells[1],User.m_lastCells[2],User.m_lastCells[3],User.m_lastCells[4],User.m_lastCells[5],User.m_lastCells[6],User.m_lastCells[7],User.m_lastCells[8],User.m_lastCells[9],User.m_lastCells[10],User.m_lastCells[11],User.m_lastCells[12],User.m_lastCells[13],User.m_lastCells[14],str,mul);
        return true;
    }
    return false;
}

bool CGameUnitTest::TestNormalGame(BYTE byDeskStation,int minmul,int maxmul)
{
    GameUserData& User = m_UserData[byDeskStation];
    auto m_pConfig = GetGradeGameConfig<GameRoomConfig>(User.m_cellScore,byDeskStation);
    while (true)
    {
        m_pConfig->OnScrollCells(0,User.m_lastCells,GAME_CELLS);
         int maLiTimes = m_pConfig->calculateMaliTimes(User.m_lastCells,GAME_CELLS);
        int freeTimes = m_pConfig->calculateFreeTimes(User.m_lastCells,GAME_CELLS);
        int jackpot = m_pConfig->isTriggerJackpotBouns(User.m_lastCells,GAME_CELLS);
        int hitCells[GAME_CELLS];
        int hitLines[GAME_LINE_COUNT];
        int64_t winScore=0;
        if (maLiTimes > 0 || freeTimes > 0||jackpot>0)
        {
            continue;
        }

        m_pConfig->OnCalculateHitLines(User.m_cellScore,winScore,m_UserData[byDeskStation].m_lastCells,
            GAME_CELLS,hitLines,GAME_LINE_COUNT,hitCells,GAME_CELLS);
        int mul=winScore*GAME_LINE/User.m_cellScore;
        if (mul>minmul&&mul<=maxmul)
        {
            GAME_LOG_INFO_FMT("%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d %d",User.m_lastCells[0],User.m_lastCells[1],User.m_lastCells[2],User.m_lastCells[3],User.m_lastCells[4],User.m_lastCells[5],User.m_lastCells[6],User.m_lastCells[7],User.m_lastCells[8],User.m_lastCells[9],User.m_lastCells[10],User.m_lastCells[11],User.m_lastCells[12],User.m_lastCells[13],User.m_lastCells[14],mul);
                return true;
        }
    }
    return false;
}