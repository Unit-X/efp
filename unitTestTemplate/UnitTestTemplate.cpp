//
// Created by Anders Cedronius on 2019-12-05.
//

#include "UnitTestTemplate.h"

void UnitTestTemplate::sendData(const std::vector<uint8_t> &subPacket) {
    if ((subPacket[0] & 0x0f) != 2) {
        unitTestFailed = true;
        unitTestActive = false;
        return;
    }
    unitTestActive = false;
    std::cout << "UnitTest " << unsigned(activeUnitTest) << " done." << std::endl;
}

void UnitTestTemplate::gotData(EdgewareFrameProtocol::framePtr &packet, EdgewareFrameContent content, bool broken, uint64_t pts, uint32_t code, uint8_t stream, uint8_t flags) {

}

bool UnitTestTemplate::waitForCompletion() {
    int breakOut = 0;
    while (unitTestActive) {
        usleep(1000 * 250); //quarter of a second
        if (breakOut++ == 10) {
            std::cout << "waitForCompletion did wait for 5 seconds. fail the test." << std::endl;
            unitTestFailed = true;
            unitTestActive = false;
        }
    }
    if (unitTestFailed) {
        std::cout << "Unit test number: " << unsigned(activeUnitTest) << " Failed." << std::endl;
        return true;
    }
    return false;
}

bool UnitTestTemplate::startUnitTest() {
    unitTestFailed = false;
    unitTestActive = false;
    EdgewareFrameMessages result;
    std::vector<uint8_t> mydata;
    uint8_t streamID=1;
    myEFPReciever = new (std::nothrow) EdgewareFrameProtocol;
    myEFPPacker = new (std::nothrow) EdgewareFrameProtocol(MTU, EdgewareFrameProtocolModeNamespace::packer);
    if (myEFPReciever == nullptr || myEFPPacker == nullptr) {
        if (myEFPReciever) delete myEFPReciever;
        if (myEFPPacker) delete myEFPPacker;
        return false;
    }
    myEFPPacker->sendCallback = std::bind(&UnitTestTemplate::sendData, this, std::placeholders::_1);
    myEFPReciever->recieveCallback = std::bind(&UnitTestTemplate::gotData, this, std::placeholders::_1, std::placeholders::_2,
                                              std::placeholders::_3, std::placeholders::_4, std::placeholders::_5, std::placeholders::_6, std::placeholders::_7);
    myEFPReciever->startUnpacker(5, 2);
    mydata.resize(MTU - myEFPPacker->geType2Size());
    unitTestActive = true;
    result = myEFPPacker->packAndSend(mydata, EdgewareFrameContent::adts,1,2,streamID,NO_FLAGS);
    if (result != EdgewareFrameMessages::noError) {
        std::cout << "Unit test number: " << unsigned(activeUnitTest) << " Failed in the packAndSend method. Error-> " << signed(result)
                  << std::endl;
        myEFPReciever->stopUnpacker();
        delete myEFPReciever;
        delete myEFPPacker;
        return false;
    }

    if (waitForCompletion()){
        myEFPReciever->stopUnpacker();
        delete myEFPReciever;
        delete myEFPPacker;
        return false;
    } else {
        myEFPReciever->stopUnpacker();
        delete myEFPReciever;
        delete myEFPPacker;
        return true;
    }
}