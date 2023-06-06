#include "StdAfx.h"
#include "ServerManage.h"
#include "../../../../sdk/include/Common/writelog.h"
#include <math.h>
#include "pb_game_100002.pb.h"
#include "SlotStrategyManager.h"
#include <cstdlib>

pb_auto_release pb_auto_var;
#define FREE_TIMES_MAX 10
log4cplus::Logger g_GameLoger;
bool InitLog = false;
/*---------------------------------------------------------------------------*/
//构造函数
CServerGameDesk::CServerGameDesk(void):NSSlotGame::CSlotGameDesk(ALL_ARGEE)
{
    m_bGameStation = GS_WAIT_ARGEE;

}
/*---------------------------------------------------------------------------*/
//析构函数
CServerGameDesk::~CServerGameDesk(void)
{
}
/*---------------------------------------------------------------------------*/
bool CServerGameDesk::InitDeskGameStation()
{
    bool ret = __super::InitDeskGameStation();
    if(ret)
    {
        if(!InitLog)
        {
            g_GameLoger = m_pDataManage->m_GameLoger;
            InitLog = true;
        }
    }
    return ret;
}
//清空该位置的数据
void  CServerGameDesk::IniUserData(BYTE byStation)
{
    m_UserData[byStation].reset();
}

void CServerGameDesk::OnRequestGameRoll(BYTE bDeskStation,int64_t betScore)
{
    OnHandleStartRoll(bDeskStation,betScore);
}

void CServerGameDesk::GetFreeGameInfo(BYTE bDeskStation,pb_game_100002::free_game_info* info)
{
    const GameUserData& user = m_UserData[bDeskStation];
    if (info!=nullptr &&UserExist(bDeskStation))
    {
        info->set_leftfreetimes(user.m_nFreeTimes);
        info->set_totalfreetimes(user.m_nTotalFreeTimes);
        info->set_totalwinscore(user.m_nTotalFreeModeWinScore);
    }
}

void CServerGameDesk::GetMaliGameInfo(BYTE bDeskStation,pb_game_100002::mali_game_info* info)
{
    const GameUserData& User = m_UserData[bDeskStation];
    auto pMali = info;
    pMali->set_times(User.m_maliTimes);

    auto m_pConfig = GetGradeGameConfig<GameRoomConfig>(User.m_cellScore,bDeskStation);

    const MaliConfig& pm = m_pConfig->getMaliCfg();
    const std::vector<int>& lst = pm.getIconList();

    auto m = lst.begin();
    while(m!=lst.end())
    {
        pMali->add_celllist(*m);
        ++m;
    }

    for (int i=0;i<4;++i)
    {
        pMali->add_centercelllist( User.m_CenterMaliIndex[i]);
    }

    auto t2m = pm.getType2Multiple();
    auto i = t2m.begin();
    while(i!=t2m.end())
    {
        auto pItem = pMali->add_itemmultiple();
        pItem->set_celltype(i->first);
        pItem->set_multiple(i->second);
        ++i;
    }

    pMali->set_multiplewithsame3( pm.getMultipleL3());
    pMali->set_multiplewithsame4( pm.getMultipleC4());

    pMali->set_betscore(User.m_cellScore);
    pMali->set_winscore(User.m_lastWinScore);
    pMali->set_totlewinscore(User.m_nTotalWinScore);
}

bool  CServerGameDesk::JoinRobot()
{
    return false;
}

/*---------------------------------------------------------------------------*/
bool	CServerGameDesk::OnTimer(UINT uTimerID)
{
    switch(uTimerID)
    {
    case  TIME_UPDATE_UserData:
        {
            OnSaveAllUserData();
        }
        break;
    case TIME_UPDATE_XiaoPond:
        {
            BroadcastJackpotData();
        }
        break;
    }
    if ((uTimerID >= TIME_MAX) || (uTimerID <= TIME_MIN))
    {
        return __super::OnTimer(uTimerID);
    }
    return true;
}
bool CServerGameDesk::HandleFrameMessage(BYTE bDeskStation, NetMessageHead * pNetHead, void * pData, UINT uSize, UINT uSocketID, bool bWatchUser)
{
    switch(pNetHead->bAssistantID)
    {
    case ASS_GM_FORCE_QUIT:		//强行退出//安全退出
        {
            return true;
        }
    }
    return __super::HandleFrameMessage( bDeskStation,  pNetHead,  pData,  uSize,  uSocketID,  bWatchUser);
}
//游戏数据包处理函数
bool CServerGameDesk::HandleNotifyMessage(BYTE bDeskStation, NetMessageHead * pNetHead, void * pData, UINT uSize, UINT uSocketID, bool bWatchUser)
{
    if(bWatchUser)
    {
        return true;
    }
    if(bDeskStation < 0 || bDeskStation >=PLAY_COUNT)
    {
        return true;
    }
    switch (pNetHead->bAssistantID)
    {
    case pb_game_100002::ass_game_roll:
        {
            pb_game_100002::cs_game_roll cr;
            if(cr.ParseFromArray(pData,uSize) == false)
            {
                return false;
            }
            OnRequestGameRoll(bDeskStation,cr.betscore());
            return true;
        }
        break;
    case pb_game_100002::ass_game_start_mali:
        {
            if (m_UserData[bDeskStation].m_nextMode == GAME_MODE_MALI)
            {
                OnRequestMaliGame(bDeskStation);
            }
            else
            {
                SendGameData(bDeskStation,pb_game_100002::ass_game_start_mali,pb_game_100002::roll_mode_is_error);
            }
            return true;
        }
        break;
    default:
        {
            return __super::HandleNotifyMessage(bDeskStation,pNetHead,pData,uSize,uSocketID,bWatchUser);;
        }
    }
}

int CServerGameDesk::SwitchBet(BYTE bDeskStation,int bet)
{
    if(GAME_MODE_GENERAL != m_UserData[bDeskStation].m_nextMode)
    {
        return pb_game_manage::switch_bet_mode_forbid;
    }
    m_UserData[bDeskStation].m_cellScore = bet;
    return 0;
}
bool CServerGameDesk::OnHandleStartRoll(BYTE byStation,int64_t iCellScore)
{

    GameUserData& User = m_UserData[byStation];

    if(!UserExist(byStation))
        return false;

    User.m_currentMode = User.m_nextMode;

    //普通模式下
    if(User.m_currentMode == GAME_MODE_GENERAL)
    {
        if(!VerifyCouldBet(byStation))
        {
            return true;
        }
        if(!CheckBetEffective(iCellScore))
        {
            SendGameData(byStation,pb_game_100002::ass_game_roll,pb_game_100002::roll_bet_invalide);
            return true;
        }

        if(GetUserGameMoney(byStation) < iCellScore)
        {
            SendGameData(byStation,pb_game_100002::ass_game_roll,pb_game_100002::roll_score_not_enough);
            return true;
        }

        if(false == UserLoseScore(byStation,iCellScore)){
            SendGameData(byStation,pb_game_100002::ass_game_roll,pb_game_100002::roll_score_not_enough);
            return true;
        }
        if (User.m_cellScore!=iCellScore)
        {
            m_UserRand[byStation].seed(time(NULL)%12345);
        }

        User.m_cellScore = iCellScore;

        OnGameModeGeneral(byStation);
    }
    else if(User.m_currentMode == GAME_MODE_MALI)
    {
        SendGameData(byStation,pb_game_100002::ass_game_roll,5);
    }
    else if(User.m_currentMode == GAME_MODE_FREE)
    {
        if (m_UserData[byStation].m_nFreeTimes > 0)
        {
            m_UserData[byStation].m_nFreeTimes -= 1;
            OnGameModeGeneral(byStation);
        }
        else
        {
            m_UserData[byStation].m_nFreeTimes = 0; // 异常
        }
    }
    return true;
}
//转，通过策略
void CServerGameDesk::trySpin(BYTE byStation,int* hitCells,int* hitLines,__int64& winScore,int& maLiTimes,int& freeTimes,bool& jackpot,int& winType)
{

    //最低倍数
    int minmulti=0;
    //最高倍数
    int maxmulti=99999;
    GameUserData& User = m_UserData[byStation];

    if(!SlotStrategyConfig::getInstance()->isInit())
    {
        this->_trySpin(byStation,hitCells,hitLines,winScore,maLiTimes,freeTimes,jackpot,winType,minmulti,maxmulti,10);
        return ;
    }

    if(GAME_MODE_FREE == User.m_currentMode)
    {
        this->_trySpin(byStation,hitCells,hitLines,winScore,maLiTimes,freeTimes,jackpot,winType,minmulti,maxmulti,10);
        return;
    }
    auto user_id = m_pUserInfo[byStation]->m_UserData.dwUserID;
    StrategyModel* sm=nullptr;
    if(m_pUserInfo[byStation]->m_UserData.bVip <= 0)
    {
        sm = SlotStrategyManager::getInstance()->getFreeStrategy(user_id);
        auto curscore = m_pUserInfo[byStation]->m_UserData.i64Money;
        curscore += m_pUserInfo[byStation]->m_UserData.i64Bank;
        maxmulti = SlotStrategyManager::getInstance()->getFreeMaxWin(NAME_ID,user_id,curscore,User.m_cellScore,User.m_notVIPUserWinTopScore);
    }
    else
    {
        //新手失败，尝试进入房间策略
        if(sm==nullptr)
        {
            m_UserData[byStation].m_entersmdifftimes = m_UserData[byStation].m_entersmdifftimes - 1;
            if(m_UserData[byStation].m_entersmdifftimes <= 0)
            {
                sm = SlotStrategyManager::getInstance()->getRoomStrategy(user_id,NAME_ID,m_pUserInfo[byStation]->m_UserData.i64Money,
                    m_UserData[byStation].m_cellScore,m_UserData[byStation].m_reductionrate);
                if(sm)
                {
                    m_UserData[byStation].m_entersmdifftimes = SlotStrategyConfig::getInstance()->difftimesroom();
                    m_UserRand[byStation].seed(m_pUserInfo[byStation]->m_UserData.dwUserID);
                }
            }
        }
    }
    if(sm==nullptr)
    {
        this->_trySpin(byStation,hitCells,hitLines,winScore,maLiTimes,freeTimes,jackpot,winType,minmulti,maxmulti,10);
        return;
    }

    if(sm->est==EST_Respin_Max||sm->est==EST_Respin_MinNoMini)
    {
        //策略触发n次重转取最大或者最小
        this->_trySpin(byStation,hitCells,hitLines,winScore,maLiTimes,freeTimes,jackpot,winType,minmulti,maxmulti,10);
        __int64 win = winScore;
        //如果取最小，但是已经是0了，不用算了
        if(sm->est!=EST_Respin_MinNoMini||win!=0)
        {
            int lastCells[GAME_CELLS];
            memcpy(lastCells,User.m_lastCells,sizeof(int)*GAME_CELLS);
            for(auto i=1;i<sm->respin_num;++i)
            {
                auto nospecial = (sm->est==EST_Respin_MinNoMini);
                this->_trySpin(byStation,hitCells,hitLines,winScore,maLiTimes,freeTimes,jackpot,winType,minmulti,maxmulti,10,nospecial);
                if(sm->est==EST_Respin_Max)
                {
                    if(winScore>win)
                    {
                        win = winScore;
                        memcpy(lastCells,User.m_lastCells,sizeof(int)*GAME_CELLS);
                    }
                }
                else
                {
                    if(winScore<win)
                    {
                        win = winScore;
                        memcpy(lastCells,User.m_lastCells,sizeof(int)*GAME_CELLS);
                    }
                    if(win==0)
                    {
                        break;
                    }
                }
            }
            memcpy(User.m_lastCells,lastCells,sizeof(int)*GAME_CELLS);
        }
        winScore = 0;
        auto m_pConfig = GetGradeGameConfig<GameRoomConfig>(User.m_cellScore,byStation);
        maLiTimes = m_pConfig->calculateMaliTimes(User.m_lastCells,GAME_CELLS);
        freeTimes = m_pConfig->calculateFreeTimes(User.m_lastCells,GAME_CELLS);
        jackpot = m_pConfig->isTriggerJackpotBouns(User.m_lastCells,GAME_CELLS);
        winType = m_pConfig->calcWinTypeByWinscore(winScore,User.m_cellScore);
        m_pConfig->OnCalculateHitLines(User.m_cellScore,winScore,m_UserData[byStation].m_lastCells,GAME_CELLS,hitLines,GAME_LINE_COUNT,hitCells,GAME_CELLS);
        return;
    }
    if(sm->est==EST_Free)
    {
        //本来应该用true，但是避免无限尝试
        auto count=1000;
        while(count>0)
        {
            //策略触发一次免费游戏
            this->_trySpin(byStation,hitCells,hitLines,winScore,maLiTimes,freeTimes,jackpot,winType,minmulti,maxmulti,10);
            if(freeTimes>0||maLiTimes>0||jackpot>0)
            {
                break;
            }

            auto m_pConfig = GetGradeGameConfig<GameRoomConfig>(User.m_cellScore,byStation);
            maLiTimes = m_pConfig->calculateMaliTimes(User.m_lastCells,GAME_CELLS);
            freeTimes = m_pConfig->calculateFreeTimes(User.m_lastCells,GAME_CELLS);
            if(freeTimes <= 0)
            {
                m_pConfig->makeEnterFreeGame(User.m_lastCells);
                freeTimes = m_pConfig->calculateFreeTimes(User.m_lastCells,GAME_CELLS);
            }
            jackpot = m_pConfig->isTriggerJackpotBouns(User.m_lastCells,GAME_CELLS);
            winType = m_pConfig->calcWinTypeByWinscore(winScore,User.m_cellScore);
            m_pConfig->OnCalculateHitLines(User.m_cellScore,winScore,m_UserData[byStation].m_lastCells,GAME_CELLS,hitLines,GAME_LINE_COUNT,hitCells,GAME_CELLS);
            break;
            --count;
        }
        return;
    }
    this->_trySpin(byStation,hitCells,hitLines,winScore,maLiTimes,freeTimes,jackpot,winType,minmulti,maxmulti,10);
}
void CServerGameDesk::_trySpin(BYTE byStation,int* hitCells,int* hitLines,__int64& winScore,int& maLiTimes,int& freeTimes,bool& jackpot,int& winType,int minmulti,int maxmulti,int curdepth,bool nospecial)
{
    --curdepth;
    bool Couldfree=false;
    bool CouldSpecialGame=false;
    bool CouldEnterjackpot = true;
    GameUserData& User = m_UserData[byStation];
    auto m_pConfig = GetGradeGameConfig<GameRoomConfig>(User.m_cellScore,byStation);
    int maxmul=0;
    bool bControl=false;
    bControl = CheckControlParams(User.m_cellScore,m_pUserInfo[byStation]->m_UserData.userBuff,Couldfree,CouldSpecialGame,maxmul);
    /*if(m_pUserInfo[byStation]->m_UserData.bVip <= 0)
    {
    CouldSpecialGame = false;
    CouldEnterjackpot = false;
    }*/

    if(maxmul > maxmulti)
    {
        maxmul = maxmulti;
    }
    for(int i = 0; i < 1000; ++i)
    {
        memset(hitCells,0,sizeof(int)*GAME_CELLS);
        memset(hitLines,0,sizeof(int)*GAME_LINE_COUNT);
        winScore = 0;
        if(GAME_MODE_FREE == User.m_currentMode)
        {
            m_pConfig->OnScrollCells(m_UserRand[byStation],3,User.m_lastCells,GAME_CELLS);
        }
        else
        {
            m_pConfig->OnScrollCells(m_UserRand[byStation],0,User.m_lastCells,GAME_CELLS);
        }

        maLiTimes = m_pConfig->calculateMaliTimes(User.m_lastCells,GAME_CELLS);
        freeTimes = m_pConfig->calculateFreeTimes(User.m_lastCells,GAME_CELLS);
        jackpot = m_pConfig->isTriggerJackpotBouns(User.m_lastCells,GAME_CELLS);
        if (maLiTimes > 0 && freeTimes > 0)
        {
            continue;
        }
        //检查 是否可以进入免费
        if(freeTimes > 0 &&(!Couldfree||nospecial))
            continue;
        //检查 是否可以进入小玛丽
        if(maLiTimes > 0 &&(!CouldSpecialGame||nospecial))
            continue;
        //检查 是否可以进奖池
        if(!CouldEnterjackpot && jackpot)
        {
            continue;
        }
        if(jackpot && (!UserCouldWinJackpot(byStation,User.m_cellScore,"jackpot")||nospecial))
            continue;
        m_pConfig->OnCalculateHitLines(User.m_cellScore,winScore,m_UserData[byStation].m_lastCells,
            GAME_CELLS,hitLines,GAME_LINE_COUNT,hitCells,GAME_CELLS);
        if (bControl&&maxmul<(winScore*1.0f/User.m_cellScore))
        {
            continue;
        }
        break;
    }
    winType = m_pConfig->calcWinTypeByWinscore(winScore,User.m_cellScore);
    if(curdepth<=0)
    {
        return;
    }
    if(User.m_cellScore==0)
    {
        //理论不存在
        return;
    }
    auto multi = winScore/(float)User.m_cellScore;
    if(multi >= minmulti && multi <= maxmulti)
    {
        return;
    }
    //不满足倍数范围，重转
    this->_trySpin(byStation,hitCells,hitLines,winScore,maLiTimes,freeTimes,jackpot,winType,minmulti,maxmulti,curdepth);
}

void	CServerGameDesk::OnGameModeGeneral(BYTE byStation)
{
    GameUserData& User = m_UserData[byStation];
    __int64 winScore = 0;
    int hitCells[GAME_CELLS];
    bool isEnterFreeGame = false;
    bool isEnterPanGoldGame = false;
    int hitLines[GAME_LINE_COUNT];
    int maLiTimes = 0;
    int freeTimes = 0;
    bool jackpot = false;
    int64_t ws = 0;
    int64_t SingleWin=0;
    int winType = GAME_WINTYPE_NOEFFECT;

    auto m_pConfig = GetGradeGameConfig<GameRoomConfig>(User.m_cellScore,byStation);

    this->trySpin(byStation,hitCells,hitLines,winScore,maLiTimes,freeTimes,jackpot,winType);
    auto user_id = m_pUserInfo[byStation]->m_UserData.dwUserID;
    //上报给策略，因为他要缓存相关数据，来判断
    SlotStrategyManager::getInstance()->onSpin(NAME_ID,user_id,User.m_cellScore,winScore,GAME_MODE_FREE == User.m_currentMode);

    User.m_nSpinWin = winScore;

    if (User.m_currentMode == GAME_MODE_GENERAL)
    {
        memcpy(User.m_lastNormalCells,User.m_lastCells,sizeof(User.m_lastNormalCells));
    }
    if(freeTimes > 0)
    {
        User.m_nFreeTimes += freeTimes;
        User.m_nTotalFreeTimes += freeTimes;
    }
    if (maLiTimes > 0)
    {
        User.m_maliTimes += maLiTimes;
    }

    if (jackpot)
    {
        WinJackpot(byStation,ws,User.m_cellScore,"jackpot");
        winType = m_pConfig->calcWinTypeByWinscore(winScore+ws,User.m_cellScore);
        User.m_nJackpotWin = ws;
        User.m_szWinJackpotName = "jackpot";
    }
    else
    {
        User.m_nJackpotWin = 0;
    }

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

    if (User.m_currentMode == GAME_MODE_FREE||User.m_nextMode == GAME_MODE_FREE)
    {
        User.m_nTotalFreeModeWinScore += winScore+ws;
    }
    SingleWin=winScore+ws;
    if(winScore > 0)
    {
        winScore = UserWinScore(byStation,winScore,User.m_cellScore);
    }
    //记录单次收益
    AddRoundWinScore(byStation,SingleWin,User.m_nextMode==GAME_MODE_GENERAL?true:false);


    pb_game_100002::sc_game_roll info;
    info.set_currentmode(User.m_currentMode);
    info.set_nextmode(User.m_nextMode);
    info.set_winscore(winScore + ws);
    info.set_userscore(GetUserGameMoney(byStation));
    info.set_wintype(winType);
    info.set_betscore(User.m_cellScore);
    info.set_jackpot(ws);
    // 免费进小玛丽
    if (User.m_currentMode==GAME_MODE_FREE && User.m_nextMode == GAME_MODE_MALI)
    {
        User.m_bFreeToMali = true;
    }else{
        User.m_bFreeToMali = false;
    }

    if (User.m_currentMode==GAME_MODE_FREE || User.m_nextMode==GAME_MODE_FREE)
    {
        auto p = info.add_free_info();
        GetFreeGameInfo(byStation,p);
    }

    User.m_lastWinScore = winScore;

    for (int i=0;i<GAME_CELLS;++i)
    {        
        info.add_resultcells(User.m_lastCells[i]);
        info.add_hitcells(hitCells[i]);
    }


    for (int i=0;i<GAME_LINE_COUNT;++i)
    {
        info.add_resultlines(hitLines[i]);
    }

    if (User.m_nextMode == GAME_MODE_MALI)
    {
        auto p = info.add_mali_info();
        User.m_nTotalWinScore = 0;
        GetMaliGameInfo(byStation,p);
        p->set_winscore(0);
    }
    SendGameDataMessageLite(byStation,&info,pb_game_100002::ass_game_roll,0);

    if (User.m_nextMode==GAME_MODE_GENERAL)
    {
        User.m_nTotalFreeModeWinScore = 0;
        User.m_nTotalFreeTimes = 0;
    }

    if (jackpot)
    {
        BroadcastJackpotData();
    }
    LogWrite(byStation);
}


bool CServerGameDesk::WriteGameData(BYTE bStation,std::ostream& os,bool& bFreeMode,std::string& modeName,__int64& bet)
{
    auto &User = m_UserData[bStation];

    if(User.m_currentMode == GAME_MODE_GENERAL)
    {
        modeName  = "普通";
    }
    else if(User.m_currentMode == GAME_MODE_FREE)
    {
        modeName = "免费";
    }
    else if(User.m_currentMode == GAME_MODE_MALI)
    {
        modeName = "小玛丽";
    }
    else
    {
        modeName = "异常模式";
    }

    if(User.m_currentMode == GAME_MODE_GENERAL||User.m_currentMode == GAME_MODE_FREE)
    {
        os<<"图标:[";
        for(int i = 0; i < GAME_CELLS; ++i)
        {
            os<<User.m_lastCells[i]<<",";
        }
        os<<"] Spin赢分:["<<(double)User.m_nSpinWin/SHOW_SCOREMUL<<"]";
    }

    os<<" 押注:["<<(double)User.m_cellScore/SHOW_SCOREMUL<<" ] ";
    if(User.m_nJackpotWin>0)
    {
        os<<" 赢奖池:["<<User.m_szWinJackpotName<<","<<(double)User.m_nJackpotWin/SHOW_SCOREMUL<<"]";
    }
    if (User.m_currentMode==GAME_MODE_MALI)
    {
        os << " 玛丽信息:{中间图标:["<< User.m_CenterMaliIndex[0]<<","
            <<User.m_CenterMaliIndex[1]<<","
            <<User.m_CenterMaliIndex[2]<<","
            <<User.m_CenterMaliIndex[3]<<"],命中:["
            <<User.m_LastMaliIndex<<"],赢分:["<<(double)User.m_nMaliWin/SHOW_SCOREMUL<<"]}";
    }

    bFreeMode = (User.m_currentMode != GAME_MODE_GENERAL);
    bet = User.m_cellScore; //押注信息，不管是否免费，都需要传入当前的押注

    os.flush();
    User.m_nMaliWin=0;
    User.m_nJackpotWin=0;
    User.m_nSpinWin=0;
    User.m_szWinJackpotName.clear();
    return true;
}

void  CServerGameDesk::onSendGameStation(BYTE bDeskStation, UINT uSocketID, bool bWatchUser)
{
    GameUserData& User = m_UserData[bDeskStation];
    pb_game_100002::sc_game_info info;

    //默认押注
    if(!CheckBetEffective(User.m_cellScore))
        User.m_cellScore = DefaultBet();
    info.set_defaultbet(User.m_cellScore);
    //奖池列表

    auto m_pConfig = GetGradeGameConfig<GameRoomConfig>(User.m_cellScore,bDeskStation);

    auto p = info.mutable_gameinfo();
    p->set_currentmode(User.m_currentMode);
    p->set_nextmode(User.m_nextMode);
    p->set_winscore(0);
    p->set_wintype(0);
    p->set_betscore(User.m_cellScore);
    p->set_userscore(GetUserGameMoney(bDeskStation));
    p->set_jackpot(0);
    info.set_gameline(GAME_LINE_COUNT);
    int Cells[GAME_CELLS]={0};
    __int64 winScore = 0;
    int hitCells[GAME_CELLS];
    int hitLines[GAME_LINE_COUNT];
    memcpy(Cells,User.m_lastNormalCells,sizeof(Cells));


    for (int i=0;i<GAME_CELLS;++i)
    {
        p->add_resultcells(Cells[i]);
    }


    m_pConfig->OnCalculateHitLines(User.m_cellScore,winScore,Cells,
        GAME_CELLS,hitLines,GAME_LINE_COUNT,hitCells,GAME_CELLS);
    for (int i=0;i<GAME_CELLS;++i)
    {        
        p->add_hitcells(hitCells[i]);
    }
    for (int i=0;i<GAME_LINE_COUNT;++i)
    {
        p->add_resultlines(hitLines[i]);
    }

    auto pMali = p->add_mali_info();
    GetMaliGameInfo(bDeskStation,pMali);
    auto pFree = p->add_free_info();
    GetFreeGameInfo(bDeskStation,pFree);
    SendGameStation(bDeskStation,uSocketID,bWatchUser,&info);
}
//获取游戏状态信息
bool CServerGameDesk::OnGetGameStation(BYTE bDeskStation, UINT uSocketID, bool bWatchUser)
{
    if(bWatchUser)
    {
        return FALSE;
    }
    onSendGameStation(bDeskStation,uSocketID,bWatchUser);
    int count = this->GetDeskPlayerNum();
    if(count == 1)
    {
        StartGame();
    }
    return TRUE;
}
/*-----------------------------------------------------------------------------------------------*/
//开始启动游戏
void	CServerGameDesk::StartGame()
{
    for(int i=0; i<PLAY_COUNT; i++)
    {
        SetUserGameState(i,USER_ARGEE);
    }
    GameBegin(ALL_ARGEE);
}
//游戏开始
bool	CServerGameDesk::GameBegin(BYTE bBeginFlag)
{
    if (__super::GameBegin(bBeginFlag)==false) 
    {
        return false;
    }

    SetTimer(TIME_UPDATE_XiaoPond,TIMER_SECONDS_JACKPORT_NOTIFY);
    SetTimer(TIME_UPDATE_UserData,TIMER_SECONDS_SaveUserData);

    return TRUE;
}
/*-----------------------------------------------------------------------------------------------*/
//重置游戏状态
bool CServerGameDesk::ReSetGameState(BYTE bLastStation)
{
    return true;
}
/*-----------------------------------------------------------------------*/
//游戏结束
bool	CServerGameDesk::GameFinish(BYTE bDeskStation, BYTE bCloseFlag)
{
    KillTimer(TIME_UPDATE_XiaoPond);

    KillTimer(TIME_UPDATE_UserData);
    switch (bCloseFlag)
    {
    case GF_NORMAL:				//游戏正常结束
    case GF_SALE:				//游戏安全结束
    case GFF_FORCE_FINISH:		//用户断线离开
        {	
            m_bGameStation	=	GS_WAIT_ARGEE;
            ReSetGameState(bCloseFlag);
            __super::GameFinish(bDeskStation,bCloseFlag);
            return true;
        }
    }
    //重置数据
    ReSetGameState(bCloseFlag);
    __super::GameFinish(bDeskStation,bCloseFlag);
    return true;
}
bool    CServerGameDesk::IsPlayGame(BYTE bDeskStation,bool isNetCut)
{
    return false;
}
/*------------------------------------------------------------------------------*/
///判断此游戏桌是否开始游戏 (因为捕鱼比较特殊 玩家离开都返回false 让玩家离开桌子 而不是断线)
bool	CServerGameDesk::IsPlayingByGameStation()
{
    return false;
}
/*------------------------------------------------------------------------------*/
//用户离开游戏桌
BYTE	CServerGameDesk::UserLeftDesk(BYTE bDeskStation, CGameUserInfo * pUserInfo,int nHandleCode)
{
    this->OnSaveUserData(bDeskStation);

    BYTE ret = __super::UserLeftDesk(bDeskStation,pUserInfo,nHandleCode);
    if(ret == ERR_GR_SIT_SUCCESS)
    {
        //清空该位置的数据
        IniUserData(bDeskStation);
        //保存奖池记录
        if(GetDeskPlayerNum() <= 0)
        {
            //结束游戏
            GameFinish(bDeskStation,GF_NORMAL);
        }
    }
    return ret;
}

__int64 CServerGameDesk::GetUserBetByStation(byte byStation){
    return m_UserData[byStation].m_cellScore;
}

/*------------------------------------------------------------------------------*/
//用户断线
bool CServerGameDesk::UserNetCut(BYTE bDeskStation, CGameUserInfo * pUserInfo)
{
    //断线也算作离开
    __super::UserLeftDesk(bDeskStation,pUserInfo);
    return true;
}
/*------------------------------------------------------------------------------*/
///用户坐到游戏桌
BYTE	CServerGameDesk::UserSitDesk(MSG_GR_S_UserSit * pUserSit, CGameUserInfo * pUserInfo)
{
    BYTE result=__super::UserSitDesk(pUserSit,pUserInfo);
    if (result == ERR_GR_SIT_SUCCESS){
        m_UserRand[pUserSit->bDeskStation].seed(pUserInfo->m_UserData.dwUserID);
        IniUserData(pUserSit->bDeskStation);
        ReadUserGameData(pUserSit->bDeskStation);
        this->m_UserData[pUserSit->bDeskStation].m_notVIPUserWinTopScore = m_UserRand[pUserSit->bDeskStation].myi_rand(SlotStrategyConfig::getInstance()->getMinWinFree(),
            SlotStrategyConfig::getInstance()->getMaxWinFree());
        this->m_UserData[pUserSit->bDeskStation].m_reductionrate =0-m_UserRand[pUserSit->bDeskStation].myi_rand(SlotStrategyConfig::getInstance()->minreductionroom(),
            SlotStrategyConfig::getInstance()->minreductionroom());
    }
    return result;
}

void CServerGameDesk::OnRequestMaliGame(BYTE byStation)
{
    GameUserData& User = m_UserData[byStation];
    auto m_pConfig = GetGradeGameConfig<GameRoomConfig>(User.m_cellScore,byStation);

    User.m_nMaliWin = 0;

    if (User.m_nextMode == GAME_MODE_MALI && User.m_maliTimes > 0)
    {
        User.m_currentMode = User.m_nextMode;

        int idx = 0;
        int c4[GAME_MALI_CENTER_ICON_COUNT] = {0};
        int hitC4[GAME_MALI_CENTER_ICON_COUNT] = {0};
        uint64_t winscore = 0;
        int64_t SingleWin=0;
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

        User.m_nMaliWin = winscore;

        m_pConfig->calcMaliHitCell(c4,GAME_MALI_CENTER_ICON_COUNT,hitC4,GAME_MALI_CENTER_ICON_COUNT,idxType);

        User.m_nTotalWinScore += winscore;
        if (User.m_bFreeToMali)
        {
            User.m_nTotalFreeModeWinScore += winscore;
        }
        if (winscore>0)
        {
            SingleWin=winscore;
            winscore = UserWinScore(byStation,winscore,User.m_cellScore);
        }

        User.m_LastMaliIndex=idx;
        memcpy(User.m_CenterMaliIndex,c4,sizeof(User.m_CenterMaliIndex));
        pb_game_100002::sc_mali_game_round_result ral;
        ral.set_resultindex(idx);
        ral.set_mali_times(User.m_maliTimes);
        ral.set_winscore(winscore);
        ral.set_wintype(winType);
        ral.set_nextmode(User.m_nextMode);
        ral.set_totlewinscore(User.m_nTotalWinScore);
        ral.set_userscore(GetUserGameMoney(byStation));
        //发送小玛丽结果给客户端
        for (int i=0;i<GAME_MALI_CENTER_ICON_COUNT;++i)
        {
            ral.add_centercells(c4[i]);
            ral.add_centercellshit(hitC4[i]);
        }
        if (User.m_nextMode==GAME_MODE_FREE)
        {
            auto pInfo = ral.add_free_info();
            GetFreeGameInfo(byStation,pInfo);
        }
        SendGameDataMessageLite(byStation,&ral,pb_game_100002::ass_game_start_mali,0);

        if (User.m_maliTimes <= 0 && User.m_bFreeToMali && User.m_nFreeTimes == 0)
        {
            User.m_nextMode = GAME_MODE_GENERAL;
            User.m_nTotalFreeModeWinScore = 0;
            User.m_nTotalFreeTimes = 0;
            User.m_bFreeToMali = false;
        }
        //记录单次收益
        AddRoundWinScore(byStation,SingleWin,User.m_nextMode==GAME_MODE_GENERAL?true:false);
        LogWrite(byStation);
    }
    else
    {
        SendGameData(byStation,pb_game_100002::ass_game_start_mali,pb_game_100002::mali_times_not_enough);
    }
}

void	CServerGameDesk::OnSaveUserData(BYTE byStation)
{
    SaveUserData(byStation);
    WriteUserGameData(byStation);
}
void	CServerGameDesk::OnSaveAllUserData()
{
    SaveAllUserData();
    int count = m_bMaxPeople;
    std::vector<CGameUserInfo*> v;
    for (int i = 0; i < count;++i)
    {
        if(m_pUserInfo[i] != nullptr && !m_pUserInfo[i]->IsRobot())
        {
            v.push_back(m_pUserInfo[i]);
        }
    }
    if(!v.empty())
    {
        for(int i = 0; i < v.size(); ++i)
        {
            CGameUserInfo* pUser = v[i];
            SlotStrategyManager::getInstance()->onSaveToDB(pUser->m_UserData.dwUserID,NAME_ID,pUser->m_UserData.bVip);
        }
    }
}

void CServerGameDesk::WriteUserGameData(BYTE byStation){
    if (!UserExist(byStation))
    {
        return;
    }

    GameUserData& User = m_UserData[byStation];
    /*if (User.m_nextMode==GAME_MODE_GENERAL)
    {
    return;
    }*/
    rapidjson::StringBuffer strBuf;
    rapidjson::Writer<rapidjson::StringBuffer> writer(strBuf);
    writer.StartObject();
    writer.Key("mode");
    writer.StartObject();
    writer.Key("cellscore");
    writer.Int64(User.m_cellScore);
    if (User.m_nextMode==GAME_MODE_FREE)
    {
        writer.Key("free");
        writer.StartObject();
        writer.Key("freetime");
        writer.Int(User.m_nFreeTimes);
        writer.Key("totalfree");
        writer.Int(User.m_nTotalFreeTimes);
        writer.Key("winscore");
        writer.Int64(User.m_nTotalFreeModeWinScore);
        //writer.Key("cellscore");
        //writer.Int64(User.m_cellScore);
        writer.Key("cell");
        writer.StartArray();
        for (int i=0;i<GAME_CELLS;i++)
        {
            writer.Int(User.m_lastNormalCells[i]);
        }
        writer.EndArray();
        writer.EndObject();
    }
    else if (User.m_nextMode==GAME_MODE_MALI)
    {
        writer.Key("mali");
        writer.StartObject();
        if (User.m_bFreeToMali)
        {
            writer.Key("freetime");
            writer.Int(User.m_nFreeTimes);
            writer.Key("totalfree");
            writer.Int(User.m_nTotalFreeTimes);
            writer.Key("winscore");
            writer.Int64(User.m_nTotalFreeModeWinScore);
        }
        writer.Key("malitime");
        writer.Int(User.m_maliTimes);
        writer.Key("malicount");
        writer.Int(User.m_maliCount);
        writer.Key("totalwin");
        writer.Int64(User.m_nTotalWinScore);
        //writer.Key("cellscore");
        //writer.Int64(User.m_cellScore);
        writer.Key("cell");
        writer.StartArray();
        for (int i=0;i<GAME_CELLS;i++)
        {
            writer.Int(User.m_lastNormalCells[i]);
        }
        writer.EndArray();
        writer.EndObject();
    }
    writer.EndObject();
    writer.EndObject();

    std::string ex_data=strBuf.GetString();
    SaveUserExtensionData(byStation,ex_data);
}

void CServerGameDesk::ReadUserGameData(BYTE byStation){
    if (!UserExist(byStation))
    {
        return;
    }
    GameUserData& User = m_UserData[byStation];
    std::string ex_data;
    if(!GetUserExtensionData(byStation,ex_data)){
        return;
    }
    if(ex_data.empty())
    {
        return;
    }

    auto p=std::make_shared<CJsonConfig>();
    if (p->LoadJsonSring(ex_data.c_str()))
    {
        auto& root=p->Root();
        if (root.HasMember("mode"))
        {
            auto& mode=root["mode"];
            if (mode.HasMember("cellscore")&&mode["cellscore"].IsInt64())
            {
                User.m_cellScore=mode["cellscore"].GetInt64();
            }
            if (mode.HasMember("free"))
            {
                auto& free=mode["free"];
                if (free.HasMember("freetime")&&free["freetime"].IsInt())
                {
                    User.m_nFreeTimes=free["freetime"].GetInt();
                }
                if (free.HasMember("totalfree")&&free["totalfree"].IsInt())
                {
                    User.m_nTotalFreeTimes=free["totalfree"].GetInt();
                }
                if (free.HasMember("cellscore")&&free["cellscore"].IsInt64())
                {
                    User.m_cellScore=free["cellscore"].GetInt64();
                }
                if (free.HasMember("cell")&&free["cell"].IsArray())
                {
                    for (int i=0;i<free["cell"].Size();i++)
                    {
                        if (!free["cell"][i].IsInt())continue;
                        User.m_lastNormalCells[i]=free["cell"][i].GetInt();
                    }
                }
                User.m_nextMode=GAME_MODE_FREE;
            }else if(mode.HasMember("mali")){
                User.m_nextMode=GAME_MODE_MALI;
                auto& mali=mode["mali"];
                if (mali.HasMember("freetime")&&mali["freetime"].IsInt())
                {
                    User.m_nFreeTimes=mali["freetime"].GetInt();
                    User.m_bFreeToMali=true;
                }
                if (mali.HasMember("totalfree")&&mali["totalfree"].IsInt())
                {
                    User.m_nTotalFreeTimes=mali["totalfree"].GetInt();
                }
                if (mali.HasMember("malitime")&&mali["malitime"].IsInt())
                {
                    User.m_maliTimes=mali["malitime"].GetInt();
                }
                if (mali.HasMember("malicount")&&mali["malicount"].IsInt())
                {
                    User.m_maliCount=mali["malicount"].GetInt();
                }
                if (mali.HasMember("cellscore")&&mali["cellscore"].IsInt64())
                {
                    User.m_cellScore=mali["cellscore"].GetInt64();
                }
                if (mali.HasMember("cell")&&mali["cell"].IsArray())
                {
                    for (int i=0;i<mali["cell"].Size();i++)
                    {
                        if (!mali["cell"][i].IsInt())continue;
                        User.m_lastNormalCells[i]=mali["cell"][i].GetInt();
                    }
                }
            }
        }
    }
    ex_data="";
    SaveUserExtensionData(byStation,ex_data);
}