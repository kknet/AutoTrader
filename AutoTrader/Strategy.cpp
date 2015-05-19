#include "stdafx.h"
#include "Strategy.h"
#include <iostream>
#include "Order.h"

Strategy::Strategy()
{
}


Strategy::~Strategy()
{
}

double Strategy::calculateK(const std::list<CThostFtdcDepthMDFieldWrapper>& data, const CThostFtdcDepthMDFieldWrapper& current, int seconds) const
{
	//datetime to timestamp
	double totalExchangePrice = current.TurnOver();
	long long totalVolume = current.Volume();

	long long leftedge = current.toTimeStamp() - seconds * 2;
	for (auto it = data.begin(); it != data.end(); it++)
	{
		if (it->toTimeStamp() > leftedge){
			totalExchangePrice += it->TurnOver();
			totalVolume += it->Volume();
		}
		else{
			break;
		}
	}

	//assert(totalVolume != 0);
	//assert(totalExchangePrice >= 0.0);

	return totalExchangePrice / totalVolume;
}


k3UpThroughK5::k3UpThroughK5()
	:m_curOrder(new Order())
{
}

k3UpThroughK5::~k3UpThroughK5()
{
}

bool k3UpThroughK5::TryInvoke(const std::list<CThostFtdcDepthMDFieldWrapper>& data, CThostFtdcDepthMDFieldWrapper& info)
{
	k3UpThroughK5TechVec* curPtr = new k3UpThroughK5TechVec(info.UUID(), info.InstrumentId());
	bool orderSingal = false;
	double k3 = calculateK(data, info, 3 * 60);
	double k5 = calculateK(data, info, 5 * 60);
	curPtr->setK3m(k3);
	curPtr->setK5m(k5);

	//assert(!data.empty());
	if (data.empty())
		return false;

	auto preNode = data.begin();

	k3UpThroughK5TechVec* prePtr = dynamic_cast<k3UpThroughK5TechVec*>(preNode->GetTechVec());
	if (prePtr){
		if (prePtr->K5m() > prePtr->K3m())
		{
			if (curPtr->K3m() > curPtr->K5m()){
				// Buy Singal
				// construct Buy Order ptr
				std::cout << "[Buy Signal]" << std::endl;
				std::cout << "LastPrice: " << info.LastPrice() << std::endl;
				//Order ord(info.InstrumentId(), info.LastPrice(), ExchangeDirection::Buy);
				m_curOrder->SetInstrumentId(info.InstrumentId());
				m_curOrder->SetRefExchangePrice(info.LastPrice());
				m_curOrder->SetExchangeDirection(ExchangeDirection::Buy);
				curPtr->SetTickType(TickType::BuyPoint);
				orderSingal = true;
			}
		}
		else{
			if (curPtr->K3m() < curPtr->K5m()){
				//Sell Singal
				// construct Sell Order ptr
				std::cout << "[Sell Signal]" << std::endl;
				std::cout << "LastPrice: " << info.LastPrice() << std::endl;
				//Order ord(info.InstrumentId(), info.LastPrice(), ExchangeDirection::Sell);
				m_curOrder->SetInstrumentId(info.InstrumentId());
				m_curOrder->SetRefExchangePrice(info.LastPrice());
				m_curOrder->SetExchangeDirection(ExchangeDirection::Sell);
				curPtr->SetTickType(TickType::SellPoint);
				orderSingal = true;
			}
		}
	}

	info.SetTechVec(curPtr);

	return orderSingal;
}
 
Order k3UpThroughK5::generateOrder(){
	return *m_curOrder;
}