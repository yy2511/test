#include "StdAfx.h"
#include "GameRoomConfig.h"
#include <iterator>
#include <iostream>
#include <regex>
#include <exception>


GameRoomConfig::GameRoomConfig(void)
{
}
GameRoomConfig::~GameRoomConfig(void)
{
}

void  GameRoomConfig::LoadOtherData(tinyxml2::XMLElement* node)
{
	__super::LoadOtherData(node);
	
    if (strcmp(node->Value(),"Mali") == 0)
    {
        m_MaliCfg.LoadFromNode(dynamic_cast<tinyxml2::XMLElement*>(node));
    }
    else if (strcmp(node->Value(),"Base") == 0)
    {
        m_BaseCfg.LoadFromNode(dynamic_cast<tinyxml2::XMLElement*>(node));
    }
}
void GameRoomConfig::makeEnterFreeGame(int* retCells)
{
	int pos[][3] = {{0,1,2},{3,4,5},{6,7,8},{9,10,11},{12,13,14}};
	for(int i = 0; i < 3; ++i)
	{
		bool isFind = false;
		for(int j = 0; j < 3 ; ++j)
		{
			int npos = pos[i][j];
			if(retCells[npos] == GAME_ITEM_BOUNS)
			{
				isFind = true;
				break;
			}
		}
		if(!isFind)
		{
			int npos = pos[i][0];
			retCells[npos] = GAME_ITEM_BOUNS;
		}
	}
}
int GameRoomConfig::calculateFreeTimes(int *retCells,int size)
{
    int count = 0;
	for(int  i = 0;i < size;++i){

		if(GAME_ITEM_BOUNS == retCells[i])
			++count;
	}
	int freetimes = 0;
	for(int i = 0;i < m_BaseCfg.feeItems.size();++i){
		if(count < m_BaseCfg.feeItems[i].count)
			break;
		freetimes = m_BaseCfg.feeItems[i].times;
	}
    return freetimes;
}

int GameRoomConfig::calculateMaliTimes(int *retCells,int size)
{
	LineDataEx* pline = nullptr;
	int malitimes = 0;
    for(int lineidx = 0;lineidx< m_cfgLines;++lineidx){
		pline = m_cfgArrayLine + lineidx;
		int  i = 0;
		for(;i<pline->m_MaxCount;++i){
			if(GAME_CELL_WILD !=  retCells[pline->m_Pos[i]])
				break;
		}
		if(i > 2){
			for (int j=0;j<m_BaseCfg.maliItems.size();j++)
			{
				if (m_BaseCfg.maliItems[j].count==i)
				{
					malitimes+=m_BaseCfg.maliItems[j].times;
					break;
				}
			}
		}
	}
    return malitimes;
}

bool GameRoomConfig::maliGameStart(int currentTiems,int& cellIdx,int* c4,int count,int betScore)
{
	//计算内圈图标
	calcMaliCells(c4);
	if(currentTiems > m_MaliCfg.m_option.size() - 1){
		cellIdx = 0;
		for(int i =0 ;i< m_MaliCfg.m_IconList.size();++i){
			if(GAME_MALI_EXIT == m_MaliCfg.m_IconList[i]){
				cellIdx = i;
				break;
			}
		}
	}else
		calcMaliHit(c4,currentTiems,cellIdx);
    return true;
}

bool GameRoomConfig::calcMaliResult(int cellIdx,int* c4,int* hits,int count,int betScore,uint64_t& winscore,int& idxType)
{
    if (GAME_MALI_CENTER_ICON_COUNT == count)
    {
        idxType = this->m_MaliCfg.m_IconList[cellIdx];

        int result  =calcMaliCells(c4,hits,count);
        
        if (result == GAME_MALI_CELL_NONE)
        {
            return calcMaliScoreWithSelectIdx(c4,count,cellIdx,betScore,winscore);
        }
        
        size_t sz = m_MaliCfg.m_IconList.size();
        if (cellIdx < sz && cellIdx >=0)
        {
            uint64_t c4_score = 0;
            uint64_t idx_score = 0;
            int center_reward = result;
            calcMaliCenterWinScoreByType(center_reward,betScore,c4_score);
            calcMaliScoreWithSelectIdx(c4,count,cellIdx,betScore,idx_score);
            winscore = c4_score + idx_score;
            return true;
        }
        else
        {
            winscore = 0;
            return false;
        }
    }
    winscore = 0;
    return false;
}

bool GameRoomConfig::calcMaliHitCell(int* c4,int count,int* hitCells,int hitCellCount,int idxType)
{
    int c = count > hitCellCount ? hitCellCount : count;
    for (int i=0;i<c;++i)
    {
        if (c4[i] == idxType)
        {
            hitCells[i] = 1;
        }else{
			hitCells[i] = 0;
		}
    }
    return true;
}

void GameRoomConfig::calcMaliHit(int c4[],int index,int& hitIndex){
	int count = 0;
	int type[GAME_MALI_CENTER_ICON_COUNT];
	int hittype = GAME_MALI_EXIT;
	int rval = m_rand.myi_rand(0,99);
	int val =0;
	vector<int> rlist;
	hitIndex = 0;

	//筛选类别
	for(int i =0;i<GAME_MALI_CENTER_ICON_COUNT;++i){
		int f = 0;
		for(; f < count;++f){
			if(type[f] == c4[i])
				break;
		}
		if(f == count){
			type[count] = c4[i];
			++count;
		}
	}

	if(CALC_PERCENT(m_MaliCfg.m_option[index].yes,rval)){
		for(int i =0;i<count;++i){
			val = 0;
			auto iter = m_MaliCfg.m_hitProb.find(type[i]);
			if(iter != m_MaliCfg.m_hitProb.end())
				val = iter->second;
			rlist.push_back(val);
		}

		int scope = 0;
		for(int i = 0;i < count;++i){
			scope += rlist[i];
			rlist[i] = scope;
		}

		if(1 > scope){
			rlist.clear();
			goto TO_NO;
		}

		rval = m_rand.myi_rand(0,scope -1);
		for(int i = 0;i < count;++i){
			if(rval < rlist[i]){
				hittype = type[i];
				break;
			}
		}

	}else if(CALC_PERCENT(m_MaliCfg.m_option[index].yes + m_MaliCfg.m_option[index].no,rval)){
TO_NO:
		for(int i = 0;i < m_MaliCfg.m_nCenterTypes.size(); ++i){
			int f = 0;
			for(; f < count;++f){
				if(type[f] == m_MaliCfg.m_nCenterTypes[i])
					break;
			}
			if(f == count)
				rlist.push_back(m_MaliCfg.m_nCenterTypes[i]);
		}

		if(rlist.size() > 0){
			rval = m_rand.myi_rand(0,rlist.size() -1);
			hittype = rlist[rval];
		}
	}

	rlist.clear();
	for(int i = 0; i <  m_MaliCfg.m_IconList.size(); ++i){
		if(hittype == m_MaliCfg.m_IconList[i])
			rlist.push_back(i);
	}

	if(rlist.size() > 0){
		rval = m_rand.myi_rand(0,rlist.size() -1);
		hitIndex = rlist[rval];
	}

}

int GameRoomConfig::calcMaliCells(int* c4,int* hits,int count)
{
	if(count == GAME_MALI_CENTER_ICON_COUNT)
	{
		if((c4[0] == c4[1]) && (c4[1] == c4[2]) && (c4[2] == c4[3]))
		{
			return GAME_MALI_CELL_C4;
		}
		else if((c4[0] == c4[1]) && (c4[1] == c4[2]))
		{
			return GAME_MALI_CELL_L3;
		}
		else if((c4[1] == c4[2]) && (c4[2] == c4[3]))
		{
			return GAME_MALI_CELL_R3;
		}
	}
	return GAME_MALI_CELL_NONE;
}

void GameRoomConfig::calcMaliCells(int c4[]){
	for(int intercept = 0;intercept< 300;++intercept){
		ZeroMemory(c4,sizeof(int) * GAME_MALI_CENTER_ICON_COUNT);
		for(int i =0;i < GAME_MALI_CENTER_ICON_COUNT;++i){
			OnScrollOneCell(4+i,c4[i]);
		}

		//过滤 四连
		if((c4[0] == c4[1])&&(c4[1] == c4[2])&&(c4[2] == c4[3]))
			continue;
		break;
	}
}

bool GameRoomConfig::calcMaliScoreWithSelectIdx(int* c4,int count,int cellIdx,int betScore,uint64_t& winscore)
{
    size_t sz = m_MaliCfg.m_IconList.size();
    if (cellIdx < sz && cellIdx >= 0)
    {
        int _type = m_MaliCfg.m_IconList[cellIdx];
        if (_type == GAME_MALI_EXIT)
        {
            return false;
        }
        bool bouns = false;
        for (int i=0;i<count;++i)
        {
            if (_type == c4[i])
            {
                bouns = true;
                break;
            }
        }

        if (bouns)
        {
            auto r = m_MaliCfg.m_Type2Multiple.find(_type);
            if (r != m_MaliCfg.m_Type2Multiple.end())
            {
                 winscore = r->second * betScore;
                 return true;
            }
        }
        return false;
    }
    return false;
}

bool GameRoomConfig::calcMaliCenterWinScoreByType(int winType,int betScore,uint64_t& winscore)
{
    if (winType == GAME_MALI_CELL_L3)
    {
        winscore = betScore * m_MaliCfg.getMultipleL3();
    }
    else if(winType == GAME_MALI_CELL_R3)
    {
        winscore = betScore * m_MaliCfg.getMultipleR3();
    }
    else if(winType == GAME_MALI_CELL_C4)
    {
        winscore = betScore * m_MaliCfg.getMultipleC4();
    }
    else
    {
        winscore = 0;
        return false;
    }
    return true;
}

bool GameRoomConfig::OnCalculateHitLines(int payCoin,__int64& winScore, int* retCells,int retCellNum,int* retHitLines,int retHitLinesNum, int* retHitCellPos,int retHitCellPosNum)
{
    if(retCellNum != m_cfgCellNumber || retHitCellPosNum != m_cfgCellNumber)
    {
        return false;
    }
    if(retHitLinesNum != m_cfgLines)
    {
        return false;
    }
    __int64 win = 0;
    for (int i=0;i<m_cfgLines;++i)
    {
        LineDataEx& _line = m_cfgArrayLine[i];
        calcOneLine(i,payCoin,win,retCells,retHitLines,retHitCellPos);
        winScore += win;
        win = 0;
    }
    return winScore > 0;
}

bool GameRoomConfig::isTriggerJackpotBouns(int *retCells,int size)
{
    int count = 0;
    for (int i=0;i<size;++i)
    {
        if (retCells[i] == GAME_ITEM_SCATTER)
        {
            ++count;
            if (count == GAME_JACKPOT_TRIGGER_TIMES)
            {
                return true;
            }
        }
    }
    return false;
}

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
int GameRoomConfig::checkOneLine(const std::vector<int>& _line, int& count, int& wildCount)
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
                auto _it = m_cfgWildNoMatchCells.begin();
                bool matched = false;
                while (_it != m_cfgWildNoMatchCells.end())
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

int GameRoomConfig::calcOneLine(int _lineIdx,
    int payCoin,
    __int64& winScore, 
    int* retCells,
    int* retHitLines, 
    int* retHitCellPos)
{
    if (_lineIdx >= 0 && _lineIdx < m_cfgLines)
    {
        LineDataEx* pLine = &(m_cfgArrayLine[_lineIdx]);
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
            int cellType = checkOneLine(l,count,wildCount);

            __int64 win = 0;
            // 计算得分，返回这条线是否可连
            auto calc = [&retHitCellPos,payCoin,this,_lineIdx](const std::vector<int>& _l,const std::vector<int>& _p,int _type,int _count,int _wildCount,__int64& _winscore)->bool
            {
                int  cellKey = _type*1000+_count;
                auto fitb = m_cfgPaytable.find(cellKey);
                __int64 score = 0;
                if(fitb != m_cfgPaytable.end())
                {
                    score = payCoin/this->m_cfgLines;
                    score = score * fitb->second;
                    _winscore = _winscore + score;
                    std::vector<int> lineCells;
                    for (int i=0;i<_count;++i)
                    {
                        retHitCellPos[_p[i]] = 1;
                        lineCells.push_back(_l[i]);
                    }

                    __int64 extraWinScore = 0;
                    
                    this->OnExtraCalculateHitLine(payCoin/m_cfgLines,score,extraWinScore,lineCells,_lineIdx);
                    _winscore = _winscore + extraWinScore;
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

MaliConfig::MaliConfig()
{
    m_MultipleC4 = 0;
    m_MultipleL3 = 0;
    m_MultipleR3 = 0;
    m_nMaxTimes = 0;
}

MaliConfig::~MaliConfig()
{
    Reset();
}

bool SplitStringWithToken(const string& token,const string& source,vector<int>& ret)
{
    try
    {
        std::tr1::regex re(token);
        auto m =  std::vector<string>(
            std::tr1::sregex_token_iterator(source.begin(), source.end(), re, -1),
            sregex_token_iterator()
            );
        std::for_each(m.begin(),m.end(),[&](const string& ps)
        {
            ret.push_back(std::stoi(ps));
        });
        return true;
    }
    catch(const std::exception& e)
    {
        std::cerr<<e.what()<<std::endl;
    }
    return false;
}
bool MaliConfig::LoadFromNode(tinyxml2::XMLElement* pNode)
{
    if (pNode != nullptr && strcmp(pNode->Value(),"Mali") == 0)
    {
        Reset();
        const char* plist = pNode->Attribute("cellsList");
        if (plist != nullptr)
        {
            SplitStringWithToken(",",plist,m_IconList);
            size_t sz = m_IconList.size();
            m_vExitIconIdx.clear();
            for (int i=0;i<sz;++i)
            {
                if (m_IconList[i] == GAME_MALI_EXIT)
                {
                    m_vExitIconIdx.push_back(i);
                }
            }
        }

        m_MultipleL3 = pNode->IntAttribute("multipleL3");
        m_MultipleR3 = pNode->IntAttribute("multipleR3");
        m_MultipleC4 = pNode->IntAttribute("multipleC4");
        m_nMaxTimes = pNode->IntAttribute("maxTimes");

        auto pChild = pNode->FirstChildElement("item");
        while (pChild != nullptr)
        {
            int pt = pChild->IntAttribute("type");
            int pm = pChild->IntAttribute("multiple");
            m_Type2Multiple.insert(std::make_pair<int,int>(pt,pm));
            pChild = pChild->NextSiblingElement("item");
        }

        auto sz = m_Type2Multiple.size();
        m_nCenterTypes.resize(sz);
        auto it = m_Type2Multiple.begin();
        for (int i=0;i<sz;++i)
        {
            m_nCenterTypes[i] = it->first;
            ++it;
        }

		m_option.clear();
		pChild = pNode->FirstChildElement("HitOption");
		if (pChild)
		{
			HitOption option;
			for(pChild = pChild->FirstChildElement("item");pChild != nullptr; pChild = pChild->NextSiblingElement("item")){
				option.yes = pChild->IntAttribute("yes");
				option.no = pChild->IntAttribute("no");
				m_option.push_back(option);
			}
		}
		
		
		pChild = pNode->FirstChildElement("HitType");
		if (pChild)
		{
			m_hitProb.clear();
			for(pChild = pChild->FirstChildElement("item");pChild != nullptr; pChild = pChild->NextSiblingElement("item")){
				m_hitProb[pChild->IntAttribute("type")] = pChild->IntAttribute("scaled");
			}
		}
		
        return true;
    }
    return false;
}

void MaliConfig::Reset()
{
    m_nCenterTypes.clear();
    m_Type2Multiple.clear();
    m_IconList.clear();
    m_MultipleL3 = 0;
    m_MultipleR3 = 0;
    m_MultipleC4 = 0;
}

bool BaseConfig::LoadFromNode(tinyxml2::XMLElement* pNode)
{
    if (pNode != nullptr)
    {
		GItem item;
		feeItems.clear();
		for(tinyxml2::XMLElement* pElem = pNode->FirstChildElement("free");pElem != NULL;pElem = pElem->NextSiblingElement("free")){
			item.count = pElem->IntAttribute("count");
			item.times = pElem->IntAttribute("times");
			feeItems.push_back(item);
		}

		maliItems.clear();
		for(tinyxml2::XMLElement* pElem = pNode->FirstChildElement("mali");pElem != NULL;pElem = pElem->NextSiblingElement("mali")){
			item.count = pElem->IntAttribute("count");
			item.times = pElem->IntAttribute("times");
			maliItems.push_back(item);
		}
        return true;
    }
    return false;
}
