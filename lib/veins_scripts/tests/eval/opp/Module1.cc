#include <string>

#include <omnetpp.h>

using namespace omnetpp;

class Module1 : public cSimpleModule
{
  protected:
    virtual void initialize() override;
    virtual void handleMessage(cMessage *msg) override;
    int par1;
    cOutVector vector1;
    simsignal_t signal1;
};

void Module1::initialize()
{
	auto* msg = new cMessage();
	scheduleAt(simTime() + SimTime(1, SIMTIME_S), msg);

	par1 = par("par1").intValue();

	recordScalar("scalar1", par1);

	vector1.setName("vector1");
	vector1.setUnit("s");
	vector1.record(7 * par1);

	signal1 = registerSignal("signal1");
	emit(signal1, 11);
}

void Module1::handleMessage(cMessage *msg)
{
	delete msg;

	vector1.record(13 * (1 + getIndex()));
	emit(signal1, 17 * (1 + getIndex()));
}

Define_Module(Module1);
