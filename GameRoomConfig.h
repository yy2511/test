#pragma once
#include "Slot243BaseConfig.h"
#include "stdint.h"
#define GAME_MODE_GENERAL			0				//普通模式
#define GAME_MODE_MALI              1               //小游戏模式
#define GAME_MODE_FREE              2               //小游戏模式

#define GAME_CELL_WILD			    9				//wild图标
#define GAME_MALI_EXIT              -1              //小游戏退出图标
#define GAME_MALI_CENTER_ICON_COUNT 4               //小游戏中间的图标个数
#define GAME_CELLS						15		//图标数
#define GAME_CELL_COLUMES				5		//列数
#define GAME_CELL_ROWS					3		//行数
#define RATE_JACKPOT_DENOMINATOR        10000

#define GAME_ITEM_BOUNS                 10          //游戏BOUNS图标
#define GAME_ITEM_SCATTER               11          //游戏SCATTER图标
#define GAME_MALI_BOUNS_COUNT           3           //连续相邻列出现的BOUNS数量，满足该数量则开启小玛丽
#define GAME_FREEMODE_TRIGGER_TIMES     3           //免费游戏触发需要的WILD数量
#define GAME_JACKPOT_TRIGGER_TIMES     3           //免费游戏触发需要的scatter数量

#define GAME_MALI_CELL_L3                0x0E            //左3相同
#define GAME_MALI_CELL_R3                0x07            //右3相同
#define GAME_MALI_CELL_C4                0x0F            //中间4个都相同
#define GAME_MALI_CELL_NONE              0x00            //没有中


#define GAME_LINE_COUNT             9

//计算百分比概率 val 是否满足 set_ratio 概率
#define CALC_PERCENT( set_ratio ,val) ((val) >= (100 - (set_ratio)))

struct HitOption{
	int yes;
	int no;
};

struct GItem{
	int count;    //计数
	int times;    //次数
};

struct GameUserData
{
    GameUserData()
    {
        reset();
    }
    bool m_bFreeToMali;                     //是否通过免费进入的小玛丽
	int	m_currentMode;						//当前模式
	int	m_nextMode;							//下一轮模式
	int	m_maliTimes;						//剩余小游戏次数
	int   m_maliCount;                       //玛丽单次转动的数量
	int64_t	m_cellScore;					//压注的钱
    int64_t m_lastWinScore;                 //上一把赢的钱
	int m_lastCells[GAME_CELLS];			//中奖图标
	int m_lastNormalCells[GAME_CELLS];		//最后一把普通图标
    __int64 m_nJackpotWin;//奖池赢分
    __int64 m_nSpinWin;   //摇奖赢分
    __int64 m_nMaliWin;   //玛丽赢分
    std::string m_szWinJackpotName;

    int m_LastMaliIndex;                    //上次中奖的索引，如果未中奖，则为 -1
    int m_CenterMaliIndex[GAME_MALI_CENTER_ICON_COUNT];           //上次中奖开奖的四个图标
    int64_t m_nTotalWinScore;               //玛丽游戏总共赢得的分

    //免费游戏
    int m_nFreeTimes;
    int m_nTotalFreeTimes;
    int m_nTotalFreeModeWinScore;           //免费游戏赢的总分
	//非充值玩家最多能赢多少
	int m_notVIPUserWinTopScore;
	//充值玩家正常玩衰减度
	int m_reductionrate;
	//
	int m_entersmdifftimes;
	//
	void reset()
	{
        m_nJackpotWin = 0;
        m_nMaliWin = 0;
        m_nSpinWin = 0;
		m_currentMode = GAME_MODE_GENERAL;
		m_nextMode = GAME_MODE_GENERAL;
        m_maliTimes = 0;
		m_maliCount = 0;
		m_nFreeTimes = false;
        m_bFreeToMali = false;
		m_cellScore = 0;
        m_LastMaliIndex = -1;
        m_lastWinScore = 0;
        m_nTotalWinScore = 0;
        m_nTotalFreeTimes = 0;
        m_nTotalFreeModeWinScore = 0;
        memset(m_CenterMaliIndex,0,sizeof(int)*GAME_MALI_CENTER_ICON_COUNT);
		memset(m_lastCells,0,sizeof(int)*GAME_CELLS);
		memset(m_lastNormalCells,0,sizeof(int)*GAME_CELLS);
		m_notVIPUserWinTopScore = 0;
		m_reductionrate = -50;
		m_entersmdifftimes = 10;
	}
};
class GameRoomConfig;
class MaliConfig
{
public:
    MaliConfig();
    ~MaliConfig();
    bool LoadFromNode(tinyxml2::XMLElement* pNode);
    int getMultipleL3()const{return m_MultipleL3;}
    int getMultipleR3()const{return m_MultipleR3;}
    int getMultipleC4()const{return m_MultipleC4;}
    const std::vector<int>& getIconList()const{return m_IconList;}
    const std::hash_map<int,int>& getType2Multiple()const {return m_Type2Multiple;}
    friend class GameRoomConfig;
protected:
    void Reset();
private:
    std::vector<int> m_IconList;
    int              m_MultipleL3;
    int              m_MultipleR3;
    int              m_MultipleC4;
    int              m_nMaxTimes;
    std::vector<int>    m_nCenterTypes;
    std::hash_map<int,int> m_Type2Multiple;
    std::vector<int> m_vExitIconIdx;

	//中奖概率控制
	std::vector<HitOption> m_option;
	std::vector<int> m_InsideCells;   //内圈图案 滚轴
	std::hash_map<int,int>m_hitProb; //外圈击中概率
};

class BaseConfig
{
public:
    friend class GameRoomConfig;
    BaseConfig()
    {
    }
    ~BaseConfig()
    {
    }
    bool LoadFromNode(tinyxml2::XMLElement* pNode);
protected:
   vector<GItem> feeItems;               //免费项
   vector<GItem> maliItems;             //玛丽项
};

class GameRoomConfig:public SlotBaseConfig
{
public:
	GameRoomConfig(void);
	~GameRoomConfig(void);
public:
	//
	virtual void  LoadOtherData(tinyxml2::XMLElement* node);
    
public:
    const MaliConfig& getMaliCfg(){return m_MaliCfg;}

	void makeEnterFreeGame(int* retCells);
    //************************************
    // Method:    isTriggerJackpotBouns
    // Desc:       是否触发了 奖金池
    // FullName:  GameRoomConfig::isTriggerJackpotBouns
    // Access:    public 
    // Returns:   int
    // Qualifier:
    // Parameter: int * retCells
    // Parameter: int size
    //************************************
    bool isTriggerJackpotBouns(int *retCells,int size);

    //************************************
    // Method:    checkOneLine
    // Desc:
    // FullName:  checkOneLine
    // Access:    public 
    // Returns:   int       返回的连续对象类型
    // Qualifier:
    // Parameter: const std::vector<int> & _line
    // Parameter: int & count       最大连续某个类型的对象
    // Parameter: int & wildCount   最大连续wild数量
    //************************************
    int checkOneLine(const std::vector<int>& _line, int& count, int& wildCount);
    //************************************
    // Method:    calculateFreeTimes
    // Desc:      计算免费游戏次数
    // FullName:  GameRoomConfig::calculateFreeTimes
    // Access:    public 
    // Returns:   int
    // Qualifier:
    // Parameter: int * retCells  图标列表
    // Parameter: int size        图标数量
    //************************************
    int calculateFreeTimes(int *retCells,int size);
    //************************************
    // Method:    calculateMaliTimes
    // Desc:      计算小玛丽的次数
    // FullName:  GameRoomConfig::calculateMaliTimes
    // Access:    public 
    // Returns:   int
    // Qualifier:
    // Parameter: int * retCells  图标列表
    // Parameter: int size        图标数量
    //************************************
    int calculateMaliTimes(int *retCells,int size);

    //************************************
    // Method:    maliGameStart
    // FullName:  GameRoomConfig::maliGameStart
    // Access:    public 
    // Returns:   bool
    // Qualifier:
    // Parameter: int leftTimes         当前剩余的小游戏次数
    // Parameter: int & cellIdx         命中的小游戏外层的图标索引，-1则为Exit图标
    // Parameter: int * c4              命中的中间的图标数组
    // Parameter: int count             命中的中间的图标数组的大小
    // Parameter: int betScore         押注分数
    //************************************
    bool maliGameStart(int currentTiems,int& cellIdx,int* c4,int count,int betScore);
    
    //************************************
    // Method:    calcMaliResult
    // Desc:      计算小游戏的得分
    // FullName:  GameRoomConfig::calcMaliResult
    // Access:    public 
    // Returns:   bool
    // Qualifier:
    // Parameter: int cellIdx           命中的小游戏外层的图标索引，-1则为Exit图标
    // Parameter: int * c4              命中的中间的图标数组
    // Parameter: int count             命中的中间的图标数组的大小
    // Parameter: int betScore         押注分数
    // Parameter: uint64_t & winscore   玩家赢得的分数
    // Parameter: int& idxType          命中的类型
    //************************************
    bool calcMaliResult(int cellIdx,int* c4,int* hits,int count,int betScore,uint64_t& winscore,int& idxType);

    //************************************
    // Method:    calcMaliHitCell
    // Desc:      计算中间图标的命中数组
    // FullName:  GameRoomConfig::calcMaliHitCell
    // Access:    public 
    // Returns:   bool
    // Qualifier:
    // Parameter: int * c4  中间图标的数组
    // Parameter: int count 中间图标的个数
    // Parameter: int * hitCells 中间图标命中的数组
    // Parameter: int hitCellCount 中间图标命中的个数
    // Parameter: int idxType 筛查的类型
    //************************************
    bool calcMaliHitCell(int* c4,int count,int* hitCells,int hitCellCount,int idxType);

    //************************************
    // Method:    calcMaliCells
    // Desc:      计算是否有中图标奖励，
    // FullName:  GameRoomConfig::calcMaliCells
    // Access:    public 
    // Returns:   int       返回最大连续icon的次数
    // Qualifier:
    // Parameter: int * c4  命中的中间的图标数组的大小
	// Parameter: int * hits  命中的中间的图标数组 参数可为空，表示不填入
    // Parameter: int count 命中的中间的图标数组的大小
    //************************************
    int calcMaliCells(int* c4,int* hits,int count);

	//************************************
    // Method:    calcMaliCells
    // Desc:      计算是否有中图标奖励，
    // FullName:  GameRoomConfig::calcMaliCells
    // Access:    public 
    // Returns:   void
    // Qualifier:
    // Parameter: int * c4  命中的中间的图标数组的大小
    //************************************
    void calcMaliCells(int* c4);

	//************************************
    // Method:    calcMaliHit
	// Desc:      计算外圈的命中
    // FullName:  GameRoomConfig::calcMaliHit
    // Access:    public 
    // Returns:   void
    // Qualifier:
    // Parameter:int c4[]          中间的图标数组 
	// Parameter:int& hitIndex         命中的外圈索引 
    //************************************
	void calcMaliHit(int c4[],int index,int& hitIndex);

    //************************************
    // Method:    calcMaliScoreWithSelectIdx
    // Desc:      通过选中的cellIndex计算活得的分数
    // FullName:  GameRoomConfig::calcMaliScoreWithSelectIdx
    // Access:    public 
    // Returns:   bool                 是否有赢钱
    // Qualifier:
    // Parameter: int * c4
    // Parameter: int count
    // Parameter: int cellIdx
    // Parameter: int betScore         押注分数
    // Parameter: uint64_t& winscore
    //************************************
    bool calcMaliScoreWithSelectIdx(int* c4,int count,int cellIdx,int betScore,uint64_t& winscore);

    //************************************
    // Method:    calcMaliCenterWinScoreByType
    // Desc:      通过中间图标的出现序列和次数计算分数
    // FullName:  GameRoomConfig::calcMaliCenterWinScoreByType
    // Access:    public 
    // Returns:   bool
    // Qualifier:
    // Parameter: int winType               中间4个图标的出现类型
    // Parameter: int betScore              押注的钱
    // Parameter: uint64_t & winscore       赢的钱
    //************************************
    bool calcMaliCenterWinScoreByType(int winType,int betScore,uint64_t& winscore);

    //************************************
    // Method:    balanceFullScreenBouns
    // Desc:      
    // FullName:  GameRoomConfig::balanceFullScreenBouns
    // Access:    public 
    // Returns:   bool 奖金池是否被掏空？true 是，false,否
    // Qualifier:
    // Parameter: int64_t betScore
    // Parameter: int64_t & winscore
    //************************************
    bool balanceFullScreenBouns(int64_t betScore,int64_t& winscore);

    //************************************
    // Method:    OnCalculateHitLines
    // Desc:      根据图标结果计算命中线和命中图标和得分
    // FullName:  GameRoomConfig::OnCalculateHitLines
    // Access:    virtual public 
    // Returns:   bool
    // Qualifier:
    // Parameter: int payCoin           押注
    // Parameter: __int64 & winScore    得分
    // Parameter: int * retCells        要计算的图标列表
    // Parameter: int retCellNum        要计算的图标列表数量
    // Parameter: int * retHitLines     命中线数的计算结果in:out
    // Parameter: int retHitLinesNum    命中线数的个数 in
    // Parameter: int * retHitCellPos   命中图标的数据 in:out
    // Parameter: int retHitCellPosNum  命中图标数据的个数 in
    //************************************
    virtual bool OnCalculateHitLines(int payCoin,__int64& winScore,
        int* retCells,int retCellNum,int* retHitLines,int retHitLinesNum,
        int* retHitCellPos,int retHitCellPosNum);
private:

    //************************************
    // Method:    calcOneLine
    // Desc:        计算单条线的数据
    // FullName:  GameRoomConfig::calcOneLine
    // Access:    private 
    // Returns:   void int wild数量 ,-1为不可计算
    // Qualifier:
    // Parameter: int _lineIdx
    // Parameter: int payCoin
    // Parameter: __int64 & winScore
    // Parameter: int * retCells
    // Parameter: int * retHitLines
    // Parameter: int * retHitCellPos
    //************************************
    int calcOneLine(int _lineIdx,int payCoin,__int64& winScore,
        int* retCells,int* retHitLines,
        int* retHitCellPos);
public:
private:
    MaliConfig      m_MaliCfg;  //小玛丽配置
    BaseConfig      m_BaseCfg;  //基础配置
};

