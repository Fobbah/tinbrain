#include "Tinbrain.h"
#include "bwlog.h"
using namespace BWAPI;

namespace BWSAL {

void Tinbrain::onStart()
{
	FILELog::ReportingLevel() = logDEBUG1;
	FILE* log_fd = fopen("tinbrain.log", "w");
	Output2FILE::Stream() = log_fd;

	BWSAL::BWSAL_init();
	Broodwar->enableFlag(Flag::UserInput);
	BWTA::readMap();
	BWTA::analyze();
	BWSAL::resetLog();

	m_eventLogger = new BWSAL::BwapiEventLogger();
	m_informationManager = InformationManager::create();
	m_borderManager = BorderManager::create(m_informationManager);
	m_baseManager = BaseManager::create(m_borderManager);
	m_buildingPlacer = new BFSBuildingPlacer();
	m_reservedMap = ReservedMap::create();
	m_unitArbitrator = new UnitArbitrator();
	m_unitGroupManager = UnitGroupManager::create();
	m_buildUnitManager = BuildUnitManager::create();
	m_buildEventTimeline = BuildEventTimeline::create(m_buildUnitManager);
	m_taskScheduler = TaskScheduler::create(m_buildEventTimeline, m_buildUnitManager);
	m_taskExecutor = TaskExecutor::create(m_unitArbitrator, m_buildEventTimeline, m_reservedMap, m_buildingPlacer);
	m_workerManager = WorkerManager::create(m_unitArbitrator, m_baseManager);
	m_scoutManager = ScoutManager::create(m_unitArbitrator, m_informationManager);
	//m_defenseManager = DefenseManager::create(m_unitArbitrator, m_borderManager);
	m_buildOrderManager = BuildOrderManager::create(m_taskScheduler, m_taskExecutor, m_buildUnitManager);
	m_supplyManager = SupplyManager::create(m_buildOrderManager, m_taskScheduler);
	m_offenseManager = new OffenseManager(m_informationManager);
	m_enhancedUI = new EnhancedUI();

	m_buildEventTimeline->initialize();
	m_scoutManager->initialize();

	m_modules.push_back(m_eventLogger);
	m_modules.push_back(m_informationManager);
	m_modules.push_back(m_borderManager);
	m_modules.push_back(m_baseManager);
	m_modules.push_back(m_buildEventTimeline);
	m_modules.push_back(m_reservedMap);
	m_modules.push_back(m_unitGroupManager);
	m_modules.push_back(m_buildUnitManager);
	m_modules.push_back(m_taskScheduler);
	m_modules.push_back(m_taskExecutor);
	m_modules.push_back(m_workerManager);
	m_modules.push_back(m_scoutManager);
	m_modules.push_back(m_offenseManager);
	//m_modules.push_back(m_defenseManager);


  BWAPI::Race race = Broodwar->self()->getRace();
  BWAPI::Race enemyRace = Broodwar->enemy()->getRace();
  BWAPI::UnitType workerType = race.getWorker();
  double minDist;
  BWTA::BaseLocation* natural = NULL;
  BWTA::BaseLocation* home = BWTA::getStartLocation(Broodwar->self());
  for (BWTA::BaseLocation* bl : BWTA::getBaseLocations())
  {
	  if (bl != home)
	  {
		  double dist = home->getGroundDistance(bl);
		  if (dist > 0)
		  {
			  if (natural == NULL || dist < minDist)
			  {
				  minDist = dist;
				  natural = bl;
			  }
		  }
	  }
  }
  //make the basic production facility

  BWTA::RectangleArray<double> sumSquaredDistanceMap(Broodwar->mapWidth(),Broodwar->mapHeight());
  sumSquaredDistanceMap.setTo(0);
  BWTA::RectangleArray<double> groundDistanceMap(Broodwar->mapWidth(),Broodwar->mapHeight());
  for(auto t = Broodwar->getStartLocations().begin();t!=Broodwar->getStartLocations().end();t++)
  {
    if (*t==Broodwar->self()->getStartLocation() && Broodwar->getStartLocations().size()>2) continue;
    BWTA::getGroundDistanceMap(*t,groundDistanceMap);
    for(int x=0;x<Broodwar->mapWidth();x++)
    {
      for(int y=0;y<Broodwar->mapHeight();y++)
      {
        double d=groundDistanceMap[x][y];
        if (d<0)
        {
          sumSquaredDistanceMap[x][y]=-1;
        }
        else
        {
          if (*t==Broodwar->self()->getStartLocation())
          {
            d*=0.5;
          }
          sumSquaredDistanceMap[x][y]+=d*d;
        }
      }
    }
  }
  TilePosition proxyLocation = TilePositions::None;
  double proxyLocationD;
  for(int x=0;x<Broodwar->mapWidth();x++)
  {
    for(int y=0;y<Broodwar->mapHeight();y++)
    {
      if (sumSquaredDistanceMap[x][y]>0)
      {
        if (proxyLocation == TilePositions::None ||
            sumSquaredDistanceMap[x][y]<proxyLocationD)
        {
          proxyLocation=TilePosition(x,y);
          proxyLocationD=sumSquaredDistanceMap[x][y];
        }
      }
    }
  }

  if (race == Races::Zerg)
  {
    int mode = rand()%6;
	m_offenseManager->setPushSize(6);
    if (mode==0)
    {
      rush_mode = "4 Pool";
	  this->m_buildOrderManager->build(4, UnitTypes::Zerg_Drone, 100);
	  this->m_buildOrderManager->buildAdditional(3, UnitTypes::Zerg_Zergling, 90);
	  this->m_buildOrderManager->buildAdditional(1, UnitTypes::Zerg_Spawning_Pool, 89);
    }
    else if (mode==1)
    {
      rush_mode = "5 Pool";
      this->m_buildOrderManager->build(5,UnitTypes::Zerg_Drone,100);
      this->m_buildOrderManager->buildAdditional(3,UnitTypes::Zerg_Zergling,90);
      this->m_buildOrderManager->buildAdditional(1,UnitTypes::Zerg_Spawning_Pool,89);
    }
    else if (mode==2)
    {
      rush_mode = "6 Pool";
      this->m_buildOrderManager->build(6,UnitTypes::Zerg_Drone,100);
      this->m_buildOrderManager->buildAdditional(3,UnitTypes::Zerg_Zergling,90);
      this->m_buildOrderManager->buildAdditional(1,UnitTypes::Zerg_Spawning_Pool,89);
    }
    else if (mode==3)
    {
      rush_mode = "7 Pool";
      this->m_buildOrderManager->build(7,UnitTypes::Zerg_Drone,100);
      this->m_buildOrderManager->buildAdditional(3,UnitTypes::Zerg_Zergling,90);
      this->m_buildOrderManager->buildAdditional(1,UnitTypes::Zerg_Spawning_Pool,89);
    }
    else if (mode==4)
    {
      rush_mode = "8 Pool";
      this->m_buildOrderManager->build(8,UnitTypes::Zerg_Drone,100);
      this->m_buildOrderManager->buildAdditional(3,UnitTypes::Zerg_Zergling,90);
      this->m_buildOrderManager->buildAdditional(1,UnitTypes::Zerg_Spawning_Pool,89);
    }
    else if (mode==5)
    {
      rush_mode = "9 Pool";
      this->m_buildOrderManager->build(9,UnitTypes::Zerg_Drone,100);
      this->m_buildOrderManager->buildAdditional(1,UnitTypes::Zerg_Overlord,95);
      this->m_buildOrderManager->buildAdditional(1,UnitTypes::Zerg_Spawning_Pool,91);
      this->m_buildOrderManager->upgrade(1,UpgradeTypes::Metabolic_Boost,90);
      this->m_buildOrderManager->buildAdditional(6,UnitTypes::Zerg_Zergling,88);
	  m_offenseManager->setPushSize(12);
    }
    this->m_buildOrderManager->buildAdditional(6,UnitTypes::Zerg_Zergling,70);
    this->m_buildOrderManager->buildAdditional(1,UnitTypes::Zerg_Overlord,50);
    this->m_buildOrderManager->buildAdditional(30,UnitTypes::Zerg_Zergling,40);
    //send an overlord out if Zerg
    this->m_scoutManager->setScoutCount(1);
  }
  else if (race == Races::Terran)
  {
    int mode = 0;//rand()%1;
    if (mode==0)
    {
      rush_mode = "Barracks Barracks Supply";
      this->m_buildOrderManager->build(8,UnitTypes::Terran_SCV,100);
      this->m_buildOrderManager->build(2,UnitTypes::Terran_Barracks,95,proxyLocation);
      this->m_buildOrderManager->build(1,UnitTypes::Terran_Supply_Depot,90);
      this->m_buildOrderManager->build(8,UnitTypes::Terran_Marine,85);
      this->m_buildOrderManager->build(2,UnitTypes::Terran_Supply_Depot,80);
      this->m_buildOrderManager->build(30,UnitTypes::Terran_Marine,75);
    }
	m_offenseManager->setPushSize(6);
  }
  else if (race == Races::Protoss)
  {
    int mode = rand()%3;
    if (mode==0)
    {
      rush_mode = "2 Gate";
      this->m_buildOrderManager->build(9,UnitTypes::Protoss_Probe,100);
      this->m_buildOrderManager->build(1,UnitTypes::Protoss_Pylon,95);
      this->m_buildOrderManager->build(2,UnitTypes::Protoss_Gateway,80);
      this->m_buildOrderManager->build(20,UnitTypes::Protoss_Zealot,75);
      m_offenseManager->setPushSize(4);
    }
    else if (mode==1)
    {
      rush_mode = "Proxy Two Gate";
      this->m_buildOrderManager->build(6,UnitTypes::Protoss_Probe,100);
      this->m_buildOrderManager->build(1,UnitTypes::Protoss_Pylon,95,proxyLocation);
      this->m_buildOrderManager->build(9,UnitTypes::Protoss_Probe,90);
      this->m_buildOrderManager->build(2,UnitTypes::Protoss_Gateway,80,proxyLocation);
      this->m_buildOrderManager->build(20,UnitTypes::Protoss_Zealot,75);
	  m_offenseManager->setPushSize(4);
    }
    else if (mode==2)
    {
      rush_mode = "DT Rush";
      this->m_buildOrderManager->build(9,UnitTypes::Protoss_Probe,100);
      this->m_buildOrderManager->build(1,UnitTypes::Protoss_Pylon,95);
      this->m_buildOrderManager->build(1,UnitTypes::Protoss_Gateway,70);
      this->m_buildOrderManager->build(10,UnitTypes::Protoss_Dark_Templar,65);
      this->m_buildOrderManager->build(3,UnitTypes::Protoss_Gateway,60);
	  m_offenseManager->setPushSize(2);
    }
    this->m_buildOrderManager->build(40,UnitTypes::Protoss_Zealot,40);
    this->m_buildOrderManager->build(4,UnitTypes::Protoss_Gateway,35);
  }
  this->m_workerManager->enableAutoBuild();
  this->m_workerManager->setAutoBuildPriority(40);
  //this->m_baseManager->setRefineryBuildPriority(30);
}

Tinbrain::~Tinbrain()
{
	delete m_offenseManager;
}
void Tinbrain::onEnd(bool isWinner)
{
  log("onEnd(%d)\n",isWinner);
}
void Tinbrain::onFrame()
{
	if (Broodwar->isPaused())
	{
		return;
	}

	m_unitArbitrator->update();
	m_buildEventTimeline->reset();

	for (BWAPI::AIModule* m : m_modules)
	{
		m->onFrame();
	}
	//m_enhancedUI->update();
	m_borderManager->draw();

	m_buildEventTimeline->m_initialState.createUnclaimedBuildUnits();
	m_buildOrderManager->onFrame();
	m_supplyManager->onFrame();

	m_buildEventTimeline->draw();
	m_buildEventTimeline->drawLarvaCounts();
	int y = -16;
	y = 50 - 16;
	m_buildOrderManager->draw(20, y);

	
  if (Broodwar->isReplay()) return;
  if (!this->analyzed) return;
  Broodwar->drawTextScreen(300,0,"%s",rush_mode.c_str());
  
  if (Broodwar->getFrameCount()>24*50)
    this->m_scoutManager->setScoutCount(1);

  auto units=Broodwar->self()->getUnits();
  if (this->showManagerAssignments)
  {
    for(auto i=units.begin();i!=units.end();i++)
    {
      if (m_unitArbitrator->hasBid(*i))
      {
        int x=(*i)->getPosition().x;
        int y=(*i)->getPosition().y;
		auto bids = m_unitArbitrator->getAllBidders(*i);
        int y_off=0;
        bool first = false;
        const char activeColor = '\x07', inactiveColor = '\x16';
        char color = activeColor;
        for(auto j=bids.begin();j!=bids.end();j++)
        {
          Broodwar->drawTextMap(x,y+y_off,"%c%s: %d",color,j->first->getName().c_str(),(int)j->second);
          y_off+=15;
          color = inactiveColor;
        }
      }
    }
  }

  UnitGroup myPylonsAndGateways = SelectAll()(Pylon,Gateway)(HitPoints,"<=",200);
  for each(auto u in myPylonsAndGateways)
  {
    Broodwar->drawCircleMap(u->getPosition().x,u->getPosition().y,20,Colors::Red);
  }
}

void Tinbrain::onUnitDestroy(BWAPI::Unit unit)
{
	m_unitArbitrator->onRemoveObject(unit);

	if (Broodwar->isReplay()) return;
	for (BWAPI::AIModule* m : m_modules)
	{
		m->onUnitDestroy(unit);
	}
}

void Tinbrain::onUnitDiscover(BWAPI::Unit unit)
{
	if (Broodwar->isReplay()) return;
	for (BWAPI::AIModule* m : m_modules)
	{
		m->onUnitDiscover(unit);
	}
}
void Tinbrain::onUnitEvade(BWAPI::Unit unit)
{
	if (Broodwar->isReplay()) return;
	for (BWAPI::AIModule* m : m_modules)
	{
		m->onUnitEvade(unit);
	}
}

void Tinbrain::onUnitHide(BWAPI::Unit unit)
{
	if (Broodwar->isReplay()) return;
	for (BWAPI::AIModule* m : m_modules)
	{
		m->onUnitHide(unit);
	}
}

void Tinbrain::onUnitMorph(BWAPI::Unit unit)
{
	if (Broodwar->isReplay()) return;
	for (BWAPI::AIModule* m : m_modules)
	{
		m->onUnitMorph(unit);
	}
}
void Tinbrain::onUnitRenegade(BWAPI::Unit unit)
{
	if (Broodwar->isReplay()) return;
	for (BWAPI::AIModule* m : m_modules)
	{
		m->onUnitRenegade(unit);
	}
}

void Tinbrain::onSendText(std::string text)
{
	if (Broodwar->isReplay()) return;
	for (BWAPI::AIModule* m : m_modules)
	{
		m->onSendText(text);
	}

	BWAPI::Broodwar->sendText(text.c_str());
}

void Tinbrain::onUnitShow(BWAPI::Unit unit)
{
	if (Broodwar->isReplay()) return;
	for (BWAPI::AIModule* m : m_modules)
	{
		m->onUnitShow(unit);
	}
}

void Tinbrain::onNukeDetect(BWAPI::Position position)
{
	if (Broodwar->isReplay()) return;
	for (BWAPI::AIModule* m : m_modules)
	{
		m->onNukeDetect(position);
	}
}

void Tinbrain::onReceiveText(BWAPI::Player player, std::string text)
{
	if (Broodwar->isReplay()) return;
	for (BWAPI::AIModule* m : m_modules)
	{
		m->onReceiveText(player, text);
	}
}

void Tinbrain::onPlayerLeft(BWAPI::Player player)
{
	if (Broodwar->isReplay()) return;
	for (BWAPI::AIModule* m : m_modules)
	{
		m->onPlayerLeft(player);
	}
}

}