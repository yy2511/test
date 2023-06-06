#pragma once

#include "SlotGameDesk.h"
#include "GameRoomConfig.h"
#include <random>
#include <stdint.h>
#include <rapidjson/prettywriter.h>
#include "GameLog.h"
//定时器 ID
#define TIME_MY_TIMER				20				//公用定时器，只有这个定时器是实际定时器，其余都是从这个定时器经过判断来的
#define MY_ELAPSE					250				// 定时器间隔
#define TIMER_COUNT					11				// 用到的定时器数目
/*---------------------------------------------------------------------*/
// 下列为游戏中使用到的定时器，由上面的TIME_MY_TIMER计数进行统一处理
#define	TIME_MIN					30				//最小计时器ID
#define	TIME_MAX					50				//最大计时器ID
/*---------------------------------------------------------------------*/
const  int TIME_UPDATE_XiaoPond =   TIMERID(0);
const  int TIME_UPDATE_UserData =   TIMERID(1);
const  int TIME_ROBOT_ROLL      =  TIMERID(2);
const  int TIME_TEST			= TIMERID(3);

//游戏结束标志定义
#define GF_NORMAL					10				//游戏正常结束
#define GF_SALE						11				//游戏安全结束
class pb_auto_release
{
public:
    ~pb_auto_release()
    {
        google::protobuf::ShutdownProtobufLibrary() ;
    }
};
extern pb_auto_release pb_auto_var;
namespace pb_game_100002
{
    class mali_game_info;
}
/*---------------------------------------------------------------------*/
//游戏桌类
class CServerGameDesk : public NSSlotGame::CSlotGameDesk
{
public:
	//构造函数
	CServerGameDesk(); 
	//析构函数
	virtual ~CServerGameDesk();
	//重载函数
public:
	//游戏开始
	virtual bool	GameBegin(BYTE bBeginFlag);
	//游戏结束
	virtual bool	GameFinish(BYTE bDeskStation, BYTE bCloseFlag);
	//判断是否正在游戏
	virtual bool    IsPlayGame(BYTE bDeskStation,bool isNetCut);
	///判断此游戏桌是否开始游戏
	virtual bool IsPlayingByGameStation();
	//游戏数据包处理函数
	virtual bool	HandleNotifyMessage(BYTE bDeskStation, NetMessageHead * pNetHead, void * pData, UINT uSize, UINT uSocketID, bool bWatchUser);
	//框架消息处理函数
	virtual bool	HandleFrameMessage(BYTE bDeskStation, NetMessageHead * pNetHead, void * pData, UINT uSize, UINT uSocketID, bool bWatchUser);
	//用户离开游戏桌
	virtual BYTE	UserLeftDesk(BYTE bDeskStation, CGameUserInfo * pUserInfo,int nHandleCode);
	///用户坐到游戏桌
	virtual BYTE	UserSitDesk(MSG_GR_S_UserSit * pUserSit, CGameUserInfo * pUserInfo);
	//用户断线
	virtual bool	UserNetCut(BYTE bDeskStation, CGameUserInfo * pUserInfo);
	//初始化桌子信息
	virtual bool	InitDeskGameStation();
	//获取游戏状态信息
	virtual bool	OnGetGameStation(BYTE bDeskStation, UINT uSocketID, bool bWatchUser);
	//重置游戏状态
	virtual bool	ReSetGameState(BYTE bLastStation);
	//定时器消息
	virtual bool	OnTimer(UINT uTimerID);
	//获取用户当前押注
	virtual __int64 GetUserBetByStation(byte byStation);

	void  onSendGameStation(BYTE bDeskStation, UINT uSocketID, bool bWatchUser);
    virtual bool WriteGameData(BYTE bStation,std::ostream& os,bool& bFreeMode,std::string& modeName,__int64& bet);
	void trySpin(BYTE byStation,int* hitCells,int* hitLines,__int64& winScore,int& maLiTimes,int& freeTimes,bool& jackpot,int& winType);
protected:
	/*
	minmulti 最低倍数，包含
	maxmulti 最高倍数，包含
	curdepth 最高深度（避免无线循环，最好十层就好了）
	*/
	void _trySpin(BYTE byStation,int* hitCells,int* hitLines,__int64& winScore,int& maLiTimes,int& freeTimes,bool& jackpot,int& winType,int minmulti,int maxmulti,int curdepth,bool nospecial=false);
protected:
	//
	bool		OnHandleStartRoll(BYTE byStation,int64_t iCellScore);
	//
	void		OnGameModeGeneral(BYTE byStation);
    // 小游戏
    void        OnRequestMaliGame(BYTE byStation);
	//
	//保存玩家数据
	void		OnSaveUserData(BYTE byStation);
	//保存所有玩家
	void		OnSaveAllUserData();
	//
	bool		JoinRobot();

    virtual int SwitchBet(BYTE bDeskStation,int bet);
    
private:
	//开始启动游戏
	void	StartGame();
private:
	//清空该位置的数据
	void	IniUserData(BYTE byStation);
public:
    protected:
        void OnRequestGameRoll(BYTE bDeskStation,int64_t betScore);
        //************************************
        // Method:    GetFreeGameInfo
        // Desc:      获取免费游戏信息
        // FullName:  CServerGameDesk::GetFreeGameInfo
        // Access:    protected 
        // Returns:   void
        // Qualifier:
        // Parameter: BYTE bDeskStation 座位号
        // Parameter: pb_game_100002::free_game_info * info 协议对象的地址
        //************************************
        void GetFreeGameInfo(BYTE bDeskStation,pb_game_100002::free_game_info* info);
        //************************************
        // Method:    GetMaliGameInfo
        // Desc:      获取小玛丽游戏的信息
        // FullName:  CServerGameDesk::GetMaliGameInfo
        // Access:    protected 
        // Returns:   void
        // Qualifier:
        // Parameter: BYTE bDeskStation 座位号
        // Parameter: pb_game_100002::mali_game_info * info 协议对象的地址
        //************************************
        void GetMaliGameInfo(BYTE bDeskStation,pb_game_100002::mali_game_info* info);
		
		//************************************
		// Method:    ReadUserGameData
		// Desc:      读取玩家游戏数据
		// FullName:  CServerGameDesk::ReadUserGameData
		// Access:    public 
		// Returns:   void
		// Qualifier:
		// Parameter: BYTE byStation
		//************************************
		void ReadUserGameData(BYTE byStation);

		//************************************
		// Method:    WriteUserGameData
		// Desc:      写入玩家游戏数据
		// FullName:  CServerGameDesk::WriteUserGameData
		// Access:    public 
		// Returns:   void
		// Qualifier:
		// Parameter: BYTE byStation
		//************************************
		void WriteUserGameData(BYTE byStation);
protected:
	GameUserData      m_UserData[PLAY_COUNT];
	MY_Rand			  m_UserRand[PLAY_COUNT];
};

/******************************************************************************************************/
