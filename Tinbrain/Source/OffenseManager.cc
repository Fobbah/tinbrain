#include "OffenseManager.h"
#include "bwlog.h"

using namespace BWAPI;

namespace BWSAL {

	void OffenseManager::onFrame()
	{
		if (Broodwar->getFrameCount() % 80 == 0)
		{
			Think();
		}
		if (m_attacking)
		{
			Broodwar->drawCircle(CoordinateType::Map, lastTarget.x, lastTarget.y, 80, Colors::Red, false);
		}

		for (Unit u : mTargets)
		{
			BWAPI::Position pos = m_informationManager->getLastPosition(u);
			Broodwar->drawCircle(CoordinateType::Map, pos.x, pos.y, 5, Colors::Orange, true);
		}
	}

	void OffenseManager::onUnitDiscover(BWAPI::Unit unit)
	{
		//Add the unit to the set if it is a building
		if (unit->getType().isBuilding() && unit->getPlayer()->isEnemy(Broodwar->self()))
		{
			mTargets.insert(unit);
		}
	}

	void OffenseManager::onUnitDestroy(BWAPI::Unit unit)
	{
		mTargets.erase(unit);
	}

	void OffenseManager::Think()
	{
		UnitGroup army = SelectAll()(canAttack)(canMove).not(isWorker);
		if (army.size() > 0)
		{
			BWLOG(logINFO) << "[OffenseManager] Info=\"THINK!\" PushSize="
				<< army.size() << "/" << m_pushSize
				<< " Attacking=" << (m_attacking ? "TRUE" : "FALSE")
				<< " TargetSetSize=" << mTargets.size();
				
			if (m_attacking == false)
			{
				if ((int)army.size() >= this->m_pushSize)
					m_attacking = true;
			}
			if (m_attacking)
			{
				BWAPI::Position attackTarget = BWAPI::Positions::None;

				if (mTargets.size() > 0)
				{
					attackTarget = getAttackTarget();
				}
				if (!isPositionValid(attackTarget))
				{
					if (m_informationManager->getEnemyBases().size() > 0)
					{
						attackTarget = (*this->m_informationManager->getEnemyBases().begin())->getPosition();
					}
				}

				if (isPositionValid(attackTarget))
				{
					army.attack(attackTarget);
					Position armyPos = army.getCenter();
					Broodwar->drawLine(CoordinateType::Map, armyPos.x, armyPos.y, attackTarget.x, attackTarget.y, Colors::Red);
						lastTarget = attackTarget;
				}
				else
				{
					BWLOG(logINFO) << "[OffenseManager] Info=\"Can't find attack position, terminating attack\"";
					m_attacking = false;
				}
			}
		}
		else
		{
			BWLOG(logINFO) << "[OffenseManager] Info=\"Can't attack, no army.\"";
		}
	}

	BWAPI::Position OffenseManager::getAttackTarget()
	{
		BWLOG(logINFO) << "[OffenseManager] Info=\"BEGIN_TARGET_DUMP\"";
		for (BWAPI::Unit u : mTargets)
		{
			BWAPI::Position pos = m_informationManager->getLastPosition(u);
			BWLOG(logINFO) << "[OffenseManager] PotentialTarget=" << m_informationManager->getType(u).toString().c_str() << " Position=" << pos.x << "@" << pos.y;
		}
		for (BWAPI::Unit u : mTargets)
		{
			BWAPI::Position pos = m_informationManager->getLastPosition(u);
			if (m_informationManager->getType(u) == UnitTypes::Resource_Vespene_Geyser)
			{
				continue;
			}
			
			if (isPositionValid(pos))
			{
				BWLOG(logINFO) << "[OffenseManager] Target=" << m_informationManager->getType(u).toString().c_str() << " Position=" << pos.x << "@" << pos.y;
				return pos;
			}
		}
		return BWAPI::Positions::None;
	}

}