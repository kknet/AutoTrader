#ifndef THOST_FTDC_DEPTH_MD_FIELD_WRAPPER_H
#define THOST_FTDC_DEPTH_MD_FIELD_WRAPPER_H

#include "ThostFtdcUserApiStruct.h"
#include <vector>
#include <string>
#include <memory>
#include "TechVec.h"

typedef std::vector<std::string> CThostFtdcDepthMDFieldDBStruct;
class DBWrapper;
class TickWrapper
{
public:
	TickWrapper(CThostFtdcDepthMarketDataField* p);
	TickWrapper(const TickWrapper& obj);
	TickWrapper& operator = (const TickWrapper& obj);
	TickWrapper(TickWrapper && obj);

	~TickWrapper();

	void serializeToDB(DBWrapper& db) const;

	// return value:(unit: half second = 500ms, time to 19700101 00:00:00 + UTC8)
	long long toTimeStamp() const;

	double TurnOver() const{
		return m_MdData.Turnover;
	}

	long long Volume() const{
		return m_MdData.Volume;
	}

	inline double LastPrice() const{
		return m_MdData.LastPrice;
	}

	inline std::string InstrumentId() const{
		return std::string(m_MdData.InstrumentID);
	}

	StrategyTech* GetTechVec() const{
		return m_techvec;
	}

	void SetTechVec(StrategyTech* p){
		m_techvec = p;
	}

	long long UUID() const {
		return m_uuid;
	}

	std::string Time() const {
		std::string ret = m_MdData.TradingDay;
		ret += " ";
		ret += m_MdData.UpdateTime;
		return ret;
	}

	static TickWrapper RecoverFromDB(const CThostFtdcDepthMDFieldDBStruct& vec);

private:
	CThostFtdcDepthMarketDataField m_MdData;
	long long m_uuid;
	static bool firstlanuch;
	bool recoveryData;
	//std::shared_ptr<TechVec> m_techvec;
public:
	StrategyTech* m_techvec;
};

#endif