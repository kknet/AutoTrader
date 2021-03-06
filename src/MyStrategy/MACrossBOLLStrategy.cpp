//#include "stdafx.h"
#include "MACrossBOLLStrategy.h"
#include "MACrossBOLLTech.h"
#include "AP_Mgr.h"
#include "Order.h"
#include <assert.h>
#include "TickWrapper.h"
#include "KData.h"
#include "BOLLTech.h"
#include "TechUtils.h"
#include <math.h>


MACrossBOLLStrategy::MACrossBOLLStrategy(size_t short_ma, size_t long_ma, size_t boll_period)
: m_curOrder(new Order())
, m_shortMA(short_ma)
, m_longMA(long_ma)
, m_bollperiod(boll_period)
{
}


MACrossBOLLStrategy::~MACrossBOLLStrategy()
{
}


OrderVec MACrossBOLLStrategy::pendingOrders() const{
	return { *m_curOrder };
}

bool MACrossBOLLStrategy::tryInvoke(const std::vector<TickWrapper>& data, TickWrapper& info){
	TickType direction = TickType::Commom;
	const size_t breakthrough_confirm_duration = 100; //50ms
	MACrossBOLLTech* curPtr = new MACrossBOLLTech(CrossStratgyType::MA, m_shortMA, m_longMA, info.UUID(), info.InstrumentId(), info.Time(), info.LastPrice());
	bool orderSingal = false;
	double short_ma = calculateK(data, info, m_shortMA * 60);
	double long_ma = calculateK(data, info, m_longMA * 60);
	BOLLTech bolltech = calculateBoll(data, info, m_bollperiod * 60);
	curPtr->setShortMA(short_ma);
	curPtr->setLongMA(long_ma);
	curPtr->setBollTech(bolltech);


	if (!data.empty()){
		if (curPtr->MAShortUpLong()){ // up
			if (!data.empty() && data.size() > 500){
				std::vector<TickWrapper>::const_reverse_iterator stoper = data.rbegin();
				std::advance(stoper, breakthrough_confirm_duration);
				for (auto it = data.rbegin(); it != stoper; it++){
					StrategyTech* prePtr = it->GetTechVec();
					// if prePtr == NULL, mean it's recovered from db, so that md is not continuous. so it's should not be singal point.
					auto prePtr_ = dynamic_cast<MACrossBOLLTech*>(prePtr);
					if (prePtr_ == NULL || prePtr_->MAShortUpLong())
					{
						// not special point
						orderSingal = false;
						break;
					}
					orderSingal = true;
				}
				//special point
				if (orderSingal){
					//update m_curOrder
					m_curOrder->SetInstrumentId(info.InstrumentId());
					m_curOrder->SetOrderType(Order::LimitPriceFOKOrder);
					m_curOrder->SetRefExchangePrice(info.LastPrice());
					m_curOrder->SetExchangeDirection(THOST_FTDC_D_Buy);
					curPtr->SetTickType(TickType::BuyPoint);
				}
			}
		}
		else{ // down
			if (!data.empty() && data.size() > 500){
				std::vector<TickWrapper>::const_reverse_iterator stoper = data.rbegin();
				std::advance(stoper, breakthrough_confirm_duration);
				for (auto it = data.rbegin(); it != stoper; it++){
					StrategyTech* prePtr = it->GetTechVec();
					auto prePtr_ = dynamic_cast<MACrossBOLLTech*>(prePtr);
					if (prePtr_ == NULL || !prePtr_->MAShortUpLong())
					{
						// not special point
						orderSingal = false;
						break;
					}
					orderSingal = true;
				}
				if (orderSingal){
					//special point
					m_curOrder->SetInstrumentId(info.InstrumentId());
					m_curOrder->SetOrderType(Order::LimitPriceFOKOrder);
					m_curOrder->SetRefExchangePrice(info.LastPrice());
					m_curOrder->SetExchangeDirection(THOST_FTDC_D_Sell);
					
					curPtr->SetTickType(TickType::SellPoint);
				}
			}
		}
	}

	info.m_techvec = curPtr;
	return orderSingal;
}

bool MACrossBOLLStrategy::tryInvoke(const std::vector<TickWrapper>& tickdata, const std::vector<KData>& data, const std::vector<TickWrapper>& curmindata, TickWrapper& info){
	TickType direction = TickType::Commom;
	const size_t breakthrough_confirm_duration = 100; //50ms
	MACrossBOLLTech* curPtr = new MACrossBOLLTech(CrossStratgyType::MA, m_shortMA, m_longMA, info.UUID(), info.InstrumentId(), info.Time(), info.LastPrice());
	bool orderSingal = false;
	KData curkdata(curmindata, 60);
	double short_ma = calculateK(data, curkdata, m_shortMA);
	double long_ma = calculateK(data, curkdata, m_longMA);
	BOLLTech bolltech = calculateBoll(data, curkdata, m_bollperiod);
	curPtr->setShortMA(short_ma);
	curPtr->setLongMA(long_ma);
	curPtr->setBollTech(bolltech);

	if (!tickdata.empty()){
		if (curPtr->MAShortUpLong())
		{ // up
			if (!tickdata.empty() && tickdata.size() > 500){
				std::vector<TickWrapper>::const_reverse_iterator stoper = tickdata.rbegin();
				std::advance(stoper, breakthrough_confirm_duration);
				for (auto it = tickdata.rbegin(); it != stoper; it++){
					StrategyTech* prePtr = it->GetTechVec();
					// if prePtr == NULL, mean it's recovered from db, so that md is not continuous. so it's should not be singal point.
					auto prePtr_ = dynamic_cast<MACrossBOLLTech*>(prePtr);
					if (prePtr_ == NULL || !prePtr_->MAShortUpLong())
					{
						// not special point
						orderSingal = false;
						break;
					}
					orderSingal = true;
				}
				//special point
				if (orderSingal){
					//update m_curOrder
					m_curOrder->SetInstrumentId(info.InstrumentId());
					m_curOrder->SetOrderType(Order::LimitPriceFOKOrder);
					m_curOrder->SetRefExchangePrice(info.LastPrice());
					m_curOrder->SetExchangeDirection(THOST_FTDC_D_Buy);
					curPtr->SetTickType(TickType::BuyPoint);
				}
			}
		}
		else
		{ // down
			if (!tickdata.empty() && tickdata.size() > 500){
				std::vector<TickWrapper>::const_reverse_iterator stoper = tickdata.rbegin();
				std::advance(stoper, breakthrough_confirm_duration);
				for (auto it = tickdata.rbegin(); it != stoper; it++){
					StrategyTech* prePtr = it->GetTechVec();
					auto prePtr_ = dynamic_cast<MACrossBOLLTech*>(prePtr);
					if (prePtr_ == NULL || prePtr_->MAShortUpLong())
					{
						// not special point
						orderSingal = false;
						break;
					}
					orderSingal = true;
				}
				if (orderSingal){
					//special point
					m_curOrder->SetInstrumentId(info.InstrumentId());
					m_curOrder->SetOrderType(Order::LimitPriceFOKOrder);
					m_curOrder->SetRefExchangePrice(info.LastPrice());
					m_curOrder->SetExchangeDirection(THOST_FTDC_D_Sell);
					curPtr->SetTickType(TickType::SellPoint);
				}
			}
		}
	}

	info.m_techvec = curPtr;
	return orderSingal;
}

BOLLTech MACrossBOLLStrategy::calculateBoll(const std::vector<TickWrapper>& data, const TickWrapper& current, size_t seconds) const{
	double ma = TechUtils::CalulateMA(data, current, seconds);
	size_t tickCount = 2 * seconds;

	double total = 0;
	long long count = 0;

	long long leftedge = current.toTimeStamp() - tickCount;
	for (auto it = data.begin(); it != data.end(); it++)
	{
		if (it->toTimeStamp() > leftedge){
			double delta = it->LastPrice() - ma;
			total += (delta * delta);
			++count;
		}
		else{
			break;
		}
	}
	double var = sqrt(total / count);

	return BOLLTech(ma, var, current.LastPrice());
}

BOLLTech MACrossBOLLStrategy::calculateBoll(const std::vector<KData>& data, const KData& current, size_t mins) const{
	double ma = TechUtils::CalulateMA(data, current, mins);

	double total = 0;
	long long count = 0;

	long long leftedge = current.Timestamp() - mins * 60 - 1;
	for (auto it = data.rbegin(); it != data.rend(); it++)
	{
		if (it->Timestamp() > leftedge){
			double delta = it->LastPrice() - ma;
			total += (delta * delta);
			++count;
		}
		else{
			break;
		}
	}
	double var = (count == 0 ? 0 : sqrt(total / count));

	return BOLLTech(ma, var, current.LastPrice());
}

double MACrossBOLLStrategy::calculateK(const std::vector<TickWrapper>& data, const TickWrapper& current, size_t seconds) const{
	return TechUtils::CalulateMA(data, current, seconds);
}

double MACrossBOLLStrategy::calculateK(const std::vector<KData>& data, const KData& current, size_t mins) const{
	return TechUtils::CalulateMA(data, current, mins);
}
