#include "BaseLib/Logger.h"
#include "MicroprocessorLib/LongTime.h"
#include "MicroprocessorLib/Voltage.h"
#include "MicroprocessorLib/VoltageConfiguration.h"
#include "MicroprocessorLib/FamilyVoltageConfiguration.h"
#include "MicroprocessorLib/FixedVoltageConfiguration.h"
#include "MicroprocessorLib/FamilyVoltageConfigurationStore.h"
#include "TestLib/Assert.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestVoltageConfigurationAppendVoltage(void)
{
	CChars	sz;

	sz.Init();
	AppendVoltage(&sz, 3.3f);
	AssertString("3.3V", sz.Text());
	sz.Clear();

	AppendVoltage(&sz, 5.0f, false);
	AssertString("5.0", sz.Text());
	sz.Clear();

	AppendVoltage(&sz, 0.35f);
	AssertString("0.4V", sz.Text());
	sz.Clear();

	AppendVoltage(&sz, -1.25f);
	AssertString("-1.3V", sz.Text());
	sz.Clear();

	AppendVoltage(&sz, NoVoltage());
	AssertString("----", sz.Text());
	sz.Clear();

	AppendVoltage(&sz, NoVoltage(), false);
	AssertString("---", sz.Text());
	sz.Kill();

	AssertFalse(IsVoltage(NoVoltage()));
	AssertTrue(IsVoltage(0.0f));
	AssertTrue(InfiniteVoltsPerTime() > 1e38f);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestVoltageConfigurationPropagation(void)
{
	CVoltageConfiguration	cConfiguration;

	AssertTrue(cConfiguration.Init(3.3f, 0.8f, 2.0f, 0.2f, 3.1f, 1000, 2001));
	AssertFloat(3.3f, cConfiguration.GetVcc(), 3);
	AssertFloat(0.8f, cConfiguration.GetLowVoltageIn(), 3);
	AssertFloat(2.0f, cConfiguration.GetHighVoltageIn(), 3);
	AssertFloat(0.2f, cConfiguration.GetLowVoltageOut(), 3);
	AssertFloat(3.1f, cConfiguration.GetHighVoltageOut(), 3);
	AssertFloat(3.1f, cConfiguration.GetVoltageOut(true), 3);
	AssertFloat(0.2f, cConfiguration.GetVoltageOut(false), 3);

	AssertLong(500, cConfiguration.GetHighToLowHoldTime());
	AssertLong(1000, cConfiguration.GetHighToLowSlewTime());
	AssertLong(1001, cConfiguration.GetLowToHighHoldTime());
	AssertLong(2001, cConfiguration.GetLowToHighSlewTime());

	AssertLong(600, cConfiguration.GetHighToDisableHoldTime());
	AssertLong(1200, cConfiguration.GetHighToDisableSlewTime());
	AssertLong(1201, cConfiguration.GetLowToDisableHoldTime());
	AssertLong(2401, cConfiguration.GetLowToDisableSlewTime());
	AssertLong(600, cConfiguration.GetHighToEnableHoldTime());
	AssertLong(1200, cConfiguration.GetHighToEnableSlewTime());
	AssertLong(1201, cConfiguration.GetLowToEnableHoldTime());
	AssertLong(2401, cConfiguration.GetLowToEnableSlewTime());

	AssertInt(TV_Undriven, cConfiguration.GetValue(NoVoltage()));
	AssertInt(TV_Low, cConfiguration.GetValue(0.0f));
	AssertInt(TV_Low, cConfiguration.GetValue(0.8f));
	AssertInt(TV_Unsettled, cConfiguration.GetValue(1.5f));
	AssertInt(TV_High, cConfiguration.GetValue(2.0f));
	AssertInt(TV_High, cConfiguration.GetValue(3.3f));

	AssertLong(1001, cConfiguration.CalculateHoldTime(3.1f, 0.2f));
	AssertLong(500, cConfiguration.CalculateHoldTime(0.2f, 3.1f));
	AssertLong(750, cConfiguration.CalculateHoldTime(1.0f, 1.0f));
	AssertLong(750, cConfiguration.CalculateHoldTime(NoVoltage(), NoVoltage()));
	AssertLong(1001, cConfiguration.CalculateHoldTime(3.1f, NoVoltage()));
	AssertFloat(0.2f, cConfiguration.CalculateStartVoltage(NoVoltage()), 3);
	AssertFloat(1.7f, cConfiguration.CalculateStartVoltage(1.7f), 3);

	AssertFloat(2.9f / 2001, cConfiguration.GetVoltsPerTimeLowToHigh(), 7);
	AssertFloat(2.9f / 1000, cConfiguration.GetVoltsPerTimeHighToLow(), 7);
	AssertLong(2250, cConfiguration.GetAverageDelayTime());
	cConfiguration.Kill();

	AssertTrue(cConfiguration.Init(3.3f, 0.8f, 2.0f, 0.2f, 3.1f, 1000, 2001, 300, 401));
	AssertLong(150, cConfiguration.GetHighToDisableHoldTime());
	AssertLong(300, cConfiguration.GetHighToDisableSlewTime());
	AssertLong(201, cConfiguration.GetLowToDisableHoldTime());
	AssertLong(401, cConfiguration.GetLowToDisableSlewTime());
	AssertLong(150, cConfiguration.GetHighToEnableHoldTime());
	AssertLong(300, cConfiguration.GetHighToEnableSlewTime());
	AssertLong(201, cConfiguration.GetLowToEnableHoldTime());
	AssertLong(401, cConfiguration.GetLowToEnableSlewTime());
	cConfiguration.Kill();

	AssertTrue(cConfiguration.Init(5.0f, 0.8f, 2.0f, 0.1f, 4.9f, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12));
	AssertLong(1, cConfiguration.GetHighToLowHoldTime());
	AssertLong(2, cConfiguration.GetHighToLowSlewTime());
	AssertLong(3, cConfiguration.GetLowToHighHoldTime());
	AssertLong(4, cConfiguration.GetLowToHighSlewTime());
	AssertLong(5, cConfiguration.GetHighToDisableHoldTime());
	AssertLong(6, cConfiguration.GetHighToDisableSlewTime());
	AssertLong(7, cConfiguration.GetLowToDisableHoldTime());
	AssertLong(8, cConfiguration.GetLowToDisableSlewTime());
	AssertLong(9, cConfiguration.GetHighToEnableHoldTime());
	AssertLong(10, cConfiguration.GetHighToEnableSlewTime());
	AssertLong(11, cConfiguration.GetLowToEnableHoldTime());
	AssertLong(12, cConfiguration.GetLowToEnableSlewTime());
	AssertFloat(0.1f, cConfiguration.GetLowVoltageOut(), 3);
	AssertFloat(4.9f, cConfiguration.GetHighVoltageOut(), 3);
	cConfiguration.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestVoltageConfigurationInvalid(void)
{
	CVoltageConfiguration	cConfiguration;
	SLogConfig				sLogConfig;

	sLogConfig = gcLogger.SetSilent();
	AssertFalse(cConfiguration.Init(3.3f, 0.8f, 2.0f, 3.1f, 3.1f, 1000, 1000));
	AssertFalse(cConfiguration.Init(3.3f, 2.0f, 0.8f, 0.2f, 3.1f, 1000, 1000));
	AssertFalse(cConfiguration.Init(3.3f, 0.8f, 2.0f, 0.2f, 3.1f, 0, 1000));
	AssertFalse(cConfiguration.Init(3.3f, 0.8f, 2.0f, 0.2f, 3.1f, 1000, 1000, 1000, 0));
	AssertFalse(cConfiguration.Init(3.3f, 0.8f, 2.0f, 0.2f, 3.1f, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 0));
	gcLogger.SetConfig(&sLogConfig);
}


//////////////////////////////////////////////////////////////////////////
//
// LVC's thresholds at 1.8V, 2.7V and 3.3V, added out of order.
//
//////////////////////////////////////////////////////////////////////////
void TestVoltageConfigurationFamily(void)
{
	CFamilyVoltageConfiguration	cFamily;
	CVoltageConfiguration		cConfiguration;
	CVoltageConfiguration*		pcConfiguration;
	SLogConfig					sLogConfig;

	cFamily.Init("LVC");
	AssertString("LVC", cFamily.GetFamily());
	AssertNull(cFamily.Get(3.3f));
	AssertFalse(IsVoltage(cFamily.GetHighVoltageIn(3.3f)));
	AssertLong(MAX_INT64, cFamily.CalculateHoldTime(3.1f, 0.2f, 3.3f));

	AssertTrue(cConfiguration.Init(3.3f, 0.8f, 2.0f, 0.2f, 3.1f, 3000, 3000));
	AssertTrue(cFamily.Add(&cConfiguration, "ttl"));
	AssertTrue(cConfiguration.Init(1.8f, 0.6f, 1.2f, 0.2f, 1.6f, 6000, 6000));
	AssertTrue(cFamily.Add(&cConfiguration, ""));
	AssertTrue(cConfiguration.Init(2.7f, 0.7f, 1.7f, 0.2f, 2.5f, 4000, 2000));
	AssertTrue(cFamily.Add(&cConfiguration, NULL));

	sLogConfig = gcLogger.SetSilent();
	AssertFalse(cFamily.Add(&cConfiguration, "CMOS"));
	gcLogger.SetConfig(&sLogConfig);

	AssertSize(3, cFamily.NumConfigurations());
	AssertFloat(1.8f, cFamily.GetConfiguration(0)->GetVcc(), 3);
	AssertFloat(2.7f, cFamily.GetConfiguration(1)->GetVcc(), 3);
	AssertFloat(3.3f, cFamily.GetConfiguration(2)->GetVcc(), 3);
	AssertNull(cFamily.GetConfiguration(3));

	pcConfiguration = cFamily.GetDefaultVoltageConfiguration(LOGIC_LEVEL_TTL);
	AssertNotNull(pcConfiguration);
	AssertFloat(3.3f, pcConfiguration->GetVcc(), 3);
	AssertNull(cFamily.GetDefaultVoltageConfiguration(LOGIC_LEVEL_CMOS));
	AssertNull(cFamily.GetDefaultVoltageConfiguration("ttl"));

	AssertFloat(2.7f, cFamily.Get(2.7f)->GetVcc(), 3);
	AssertNull(cFamily.Get(3.0f));
	AssertFloat(3.3f, cFamily.GetEqualOrHigherConfiguration(3.0f)->GetVcc(), 3);
	AssertFloat(2.7f, cFamily.GetEqualOrHigherConfiguration(2.7f)->GetVcc(), 3);
	AssertNull(cFamily.GetEqualOrHigherConfiguration(3.4f));
	AssertFloat(2.7f, cFamily.GetLowerConfiguration(3.0f)->GetVcc(), 3);
	AssertFloat(1.8f, cFamily.GetLowerConfiguration(2.7f)->GetVcc(), 3);
	AssertNull(cFamily.GetLowerConfiguration(1.8f));
	AssertFloat(3.3f, cFamily.GetLowerConfiguration(5.0f)->GetVcc(), 3);

	//At a configuration's VCC.
	AssertFloat(2.0f, cFamily.GetHighVoltageIn(3.3f), 3);
	AssertFloat(0.7f, cFamily.GetLowVoltageIn(2.7f), 3);
	AssertFloat(1.6f, cFamily.GetVoltageOut(true, 1.8f), 3);

	//Between configurations.
	AssertFloat(1.85f, cFamily.GetHighVoltageIn(3.0f), 3);
	AssertFloat(0.75f, cFamily.GetLowVoltageIn(3.0f), 3);
	AssertFloat(2.8f, cFamily.GetVoltageOut(true, 3.0f), 3);
	AssertFloat(0.2f, cFamily.GetVoltageOut(false, 3.0f), 3);

	//Below the lowest, from zero, and above the highest, scaled.
	AssertFloat(0.6f, cFamily.GetHighVoltageIn(0.9f), 3);
	AssertFloat(0.8f, cFamily.GetVoltageOut(true, 0.9f), 3);
	AssertFloat(2.0f * 5.0f / 3.3f, cFamily.GetHighVoltageIn(5.0f), 3);
	AssertFloat(0.8f * 5.0f / 3.3f, cFamily.GetLowVoltageIn(5.0f), 3);

	//No power.
	AssertFalse(IsVoltage(cFamily.GetHighVoltageIn(0.0f)));
	AssertFalse(IsVoltage(cFamily.GetLowVoltageIn(NoVoltage())));
	AssertFalse(IsVoltage(cFamily.GetVoltageOut(true, 0.0f)));

	AssertInt(TV_High, cFamily.GetValue(1.9f, 3.0f));
	AssertInt(TV_Unsettled, cFamily.GetValue(1.8f, 3.0f));
	AssertInt(TV_Low, cFamily.GetValue(0.75f, 3.0f));
	AssertInt(TV_Undriven, cFamily.GetValue(NoVoltage(), 3.0f));
	AssertInt(TV_Undriven, cFamily.GetValue(1.9f, 0.0f));

	AssertFloat(0.2f, cFamily.CalculateStartVoltage(NoVoltage(), 3.0f), 3);
	AssertFloat(1.1f, cFamily.CalculateStartVoltage(1.1f, 3.0f), 3);

	//Hold times: 3.3V is 1500 both ways, 2.7V 2000 high to low and 1000 low to high.
	AssertLong(1500, cFamily.CalculateHoldTime(3.1f, 0.2f, 3.3f));
	AssertLong(1250, cFamily.CalculateHoldTime(3.1f, 0.2f, 3.0f));
	AssertLong(1750, cFamily.CalculateHoldTime(0.2f, 3.1f, 3.0f));
	AssertLong(MAX_INT64, cFamily.CalculateHoldTime(3.1f, 0.2f, 1.0f));
	AssertLong(MAX_INT64, cFamily.CalculateHoldTime(3.1f, 0.2f, 5.0f));
	AssertLong(MAX_INT64, cFamily.CalculateHoldTime(3.1f, 0.2f, 0.0f));

	AssertFloat(2.9f / 3000, cFamily.GetVoltsPerTimeLowToHigh(3.3f), 7);
	AssertFloat((2.9f / 3000 + 2.3f / 2000) / 2, cFamily.GetVoltsPerTimeLowToHigh(3.0f), 7);
	AssertFloat((2.9f / 3000 + 2.3f / 4000) / 2, cFamily.GetVoltsPerTimeHighToLow(3.0f), 7);
	AssertTrue(cFamily.GetVoltsPerTimeLowToHigh(5.0f) == InfiniteVoltsPerTime());
	AssertTrue(cFamily.GetVoltsPerTimeHighToLow(1.0f) == InfiniteVoltsPerTime());
	AssertTrue(cFamily.GetVoltsPerTimeHighToLow(0.0f) == InfiniteVoltsPerTime());

	cFamily.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestVoltageConfigurationFixed(void)
{
	CFixedVoltageConfiguration	cFixed;
	CVoltageConfiguration		cConfiguration;

	AssertTrue(cFixed.Init(3.3f, 0.8f, 2.0f, 0.2f, 3.1f, 1000, 2001));
	AssertFloat(3.3f, cFixed.Get(5.0f)->GetVcc(), 3);
	AssertFloat(0.8f, cFixed.GetLowVoltageIn(5.0f), 3);
	AssertFloat(2.0f, cFixed.GetHighVoltageIn(0.0f), 3);
	AssertFloat(3.1f, cFixed.GetVoltageOut(true, 1.0f), 3);
	AssertInt(TV_Unsettled, cFixed.GetValue(1.5f, 0.0f));
	AssertFloat(0.2f, cFixed.CalculateStartVoltage(NoVoltage(), 0.0f), 3);
	AssertFloat(2.9f / 2001, cFixed.GetVoltsPerTimeLowToHigh(0.0f), 7);
	AssertFloat(2.9f / 1000, cFixed.GetVoltsPerTimeHighToLow(0.0f), 7);
	AssertLong(1001, cFixed.CalculateHoldTime(3.1f, 0.2f, 0.0f));
	cFixed.Kill();

	AssertTrue(cConfiguration.Init(5.0f, 0.8f, 2.0f, 0.1f, 4.9f, 100, 100));
	cFixed.Init(&cConfiguration);
	AssertFloat(4.9f, cFixed.GetVoltageOut(true, 3.3f), 3);
	AssertLong(50, cFixed.CalculateHoldTime(0.1f, 4.9f, 3.3f));
	cFixed.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// Input/PropagationTimes.csv is Logicim's Propagation Times (541).csv.
//
//////////////////////////////////////////////////////////////////////////
void TestVoltageConfigurationStoreLoad(void)
{
	CFamilyVoltageConfigurationStore	cStore;
	CFamilyVoltageConfiguration*		pcFamily;
	CVoltageConfiguration*				pcConfiguration;
	SLogConfig							sLogConfig;

	cStore.Init();
	AssertTrue(cStore.Load("Input/PropagationTimes.csv"));

	AssertSize(14, cStore.NumFamilies());
	AssertString("LVC", cStore.GetFamily(0)->GetFamily());
	AssertString("HCT", cStore.GetFamily(1)->GetFamily());
	AssertString("LCX", cStore.GetFamily(13)->GetFamily());
	AssertNull(cStore.GetFamily(14));
	AssertNull(cStore.Get("CD4000"));

	pcFamily = cStore.GetDefaultFamily();
	AssertNotNull(pcFamily);
	AssertString(DEFAULT_FAMILY, pcFamily->GetFamily());
	AssertSize(3, pcFamily->NumConfigurations());

	//"LVC",3.3,"TTL","tPD",1.0,2.9,5.1,2.0,0.8,3.1,0.2 with tEN 3.5 and tDIS 3.3.
	pcConfiguration = pcFamily->GetDefaultVoltageConfiguration(LOGIC_LEVEL_TTL);
	AssertNotNull(pcConfiguration);
	AssertFloat(3.3f, pcConfiguration->GetVcc(), 3);
	AssertFloat(0.8f, pcConfiguration->GetLowVoltageIn(), 3);
	AssertFloat(2.0f, pcConfiguration->GetHighVoltageIn(), 3);
	AssertFloat(0.2f, pcConfiguration->GetLowVoltageOut(), 3);
	AssertFloat(3.1f, pcConfiguration->GetHighVoltageOut(), 3);
	AssertLong(1484, pcConfiguration->GetHighToLowHoldTime());
	AssertLong(2969, pcConfiguration->GetHighToLowSlewTime());
	AssertLong(1484, pcConfiguration->GetLowToHighHoldTime());
	AssertLong(2969, pcConfiguration->GetLowToHighSlewTime());
	AssertLong(1792, pcConfiguration->GetHighToEnableHoldTime());
	AssertLong(3584, pcConfiguration->GetLowToEnableSlewTime());
	AssertLong(1689, pcConfiguration->GetHighToDisableHoldTime());
	AssertLong(3379, pcConfiguration->GetLowToDisableSlewTime());

	//"LVC",1.8,,"tPD",1.5,6.5 with tEN 7.7 and tDIS 4.9.
	pcConfiguration = pcFamily->Get(1.8f);
	AssertNotNull(pcConfiguration);
	AssertFloat(1.2f, pcConfiguration->GetHighVoltageIn(), 3);
	AssertLong(3328, pcConfiguration->GetHighToLowHoldTime());
	AssertLong(6656, pcConfiguration->GetLowToHighSlewTime());
	AssertLong(7884, pcConfiguration->GetHighToEnableSlewTime());
	AssertLong(2508, pcConfiguration->GetLowToDisableHoldTime());
	AssertNull(pcFamily->GetDefaultVoltageConfiguration(LOGIC_LEVEL_CMOS));

	//LVT's TTL row is at 2.7V; Logicim would carry it on to 3.3V.
	pcFamily = cStore.Get("LVT");
	AssertNotNull(pcFamily);
	AssertFloat(2.7f, pcFamily->GetDefaultVoltageConfiguration(LOGIC_LEVEL_TTL)->GetVcc(), 3);

	pcFamily = cStore.Get("VCX");
	AssertFloat(1.8f, pcFamily->GetConfiguration(0)->GetVcc(), 3);
	AssertFloat(2.5f, pcFamily->GetConfiguration(1)->GetVcc(), 3);
	AssertFloat(3.3f, pcFamily->GetConfiguration(2)->GetVcc(), 3);

	sLogConfig = gcLogger.SetSilent();
	AssertFalse(cStore.Load("Input/PropagationTimes.csv"));
	AssertFalse(cStore.Load("Input/NoSuchPropagationTimes.csv"));
	gcLogger.SetConfig(&sLogConfig);
	AssertSize(14, cStore.NumFamilies());

	cStore.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestVoltageConfigurationStoreCreate(void)
{
	CArrayVoltagePropagationTimeRow		asRows;
	CArrayVoltagePropagationTimeRowPtr	apsRows;
	SVoltagePropagationTimeRow*			psRow;
	CVoltageConfiguration				cConfiguration;
	SLogConfig							sLogConfig;
	size								i;

	asRows.Init();
	apsRows.Init();

	psRow = asRows.Add();
	psRow->Init();
	psRow->szDelayType.Set("tPHL");
	psRow->fTypicalDelay = 2.0f;
	psRow->fVIH = 2.0f;
	psRow->fVIL = 0.8f;

	psRow = asRows.Add();
	psRow->Init();
	psRow->szDelayType.Set("tPLH");
	psRow->fTypicalDelay = 4.0f;
	psRow->fVOH = 4.9f;
	psRow->fVOL = 0.1f;

	psRow = asRows.Add();
	psRow->Init();
	psRow->szDelayType.Set("tEN");
	psRow->fTypicalDelay = 6.0f;

	for (i = 0; i < asRows.NumElements(); i++)
	{
		apsRows.Add(asRows.Get(i));
	}

	sLogConfig = gcLogger.SetSilent();
	AssertFalse(CFamilyVoltageConfigurationStore::CreateVoltageConfiguration(&cConfiguration, 5.0f, &apsRows));
	gcLogger.SetConfig(&sLogConfig);

	psRow = asRows.Add();
	psRow->Init();
	psRow->szDelayType.Set("tPHZ");
	psRow->fTypicalDelay = 8.0f;
	psRow = asRows.Add();
	psRow->Init();
	psRow->szDelayType.Set("tPLZ");
	psRow->fTypicalDelay = 10.0f;

	apsRows.ReInit();
	for (i = 0; i < asRows.NumElements(); i++)
	{
		apsRows.Add(asRows.Get(i));
	}

	AssertTrue(CFamilyVoltageConfigurationStore::CreateVoltageConfiguration(&cConfiguration, 5.0f, &apsRows));
	AssertFloat(5.0f, cConfiguration.GetVcc(), 3);
	AssertFloat(0.8f, cConfiguration.GetLowVoltageIn(), 3);
	AssertFloat(2.0f, cConfiguration.GetHighVoltageIn(), 3);
	AssertFloat(0.1f, cConfiguration.GetLowVoltageOut(), 3);
	AssertFloat(4.9f, cConfiguration.GetHighVoltageOut(), 3);
	AssertLong(1024, cConfiguration.GetHighToLowHoldTime());
	AssertLong(2048, cConfiguration.GetHighToLowSlewTime());
	AssertLong(2048, cConfiguration.GetLowToHighHoldTime());
	AssertLong(4096, cConfiguration.GetLowToHighSlewTime());
	AssertLong(3072, cConfiguration.GetHighToEnableHoldTime());
	AssertLong(6144, cConfiguration.GetLowToEnableSlewTime());
	AssertLong(4096, cConfiguration.GetHighToDisableHoldTime());
	AssertLong(8192, cConfiguration.GetHighToDisableSlewTime());
	AssertLong(5120, cConfiguration.GetLowToDisableHoldTime());
	AssertLong(10240, cConfiguration.GetLowToDisableSlewTime());

	apsRows.Kill();
	for (i = 0; i < asRows.NumElements(); i++)
	{
		asRows.Get(i)->Kill();
	}
	asRows.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestVoltageConfiguration(void)
{
	BeginTests();

	TestVoltageConfigurationAppendVoltage();
	TestVoltageConfigurationPropagation();
	TestVoltageConfigurationInvalid();
	TestVoltageConfigurationFamily();
	TestVoltageConfigurationFixed();
	TestVoltageConfigurationStoreLoad();
	TestVoltageConfigurationStoreCreate();

	TestStatistics();
}

