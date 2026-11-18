#!/usr/bin/env python3
"""Compile the actual SX1262 RX methods against an IRQ/SPI fake."""
from pathlib import Path
import subprocess
import tempfile
ROOT = Path(__file__).resolve().parents[1]
source = (ROOT / 'src/core/radio/SX1262.cpp').read_text()
start = source.index('bool SX1262::isRxBusy()')
end = source.index('int IRAM_ATTR SX1262::available()', start)
harness = r'''
#include <cstdint>
#include <cassert>
#include <cstring>
#include "RadioConstants.h"
#include "SX1262Timing.h"
#define MAX_PACKET_SIZE 255
struct { template<class... T> void printf(const char*, T...) {} void println() {} } Serial;
static uint32_t clock_ms;
uint32_t millis() { return clock_ms; }
struct SX1262 {
 bool _rxActivityTracking=false; uint32_t _rxActivitySince=0;
 uint8_t _sf=8, _bw=3; uint16_t _preambleLength=18;
 bool _implicitHeaderMode=false, _crcMode=true, _ldro=false;
 uint8_t getCodingRate4() { return 7; }
 bool isRxBusy();
 bool _ioFailed=false, _txActive=false, packetAvailable=false;
 int _fifo_rx_addr_ptr=0, _packetIndex=0;
 uint8_t _packet[255]={}, irq=2, nextIrq=0, status=0x52;
 uint8_t len=17;
 unsigned restarts=0, reads=0;
 void resumeReceive(); int parsePacket(int size=0);
 void receive() { ++restarts; irq=0; packetAvailable=false; status=0x52; }
 uint8_t getStatus() { return status; }
 void executeOpcodeRead(int op, uint8_t* b, int n) {
   memset(b,0,n);
   if (op==OP_GET_IRQ_STATUS_6X) { ++reads; b[1]=irq; }
   else if (op==OP_RX_BUFFER_STATUS_6X) { b[0]=len; }
   else assert(op==OP_PACKET_STATUS_6X);
 }
 void executeOpcode(int op, uint8_t* b, int n) {
   assert(op==OP_CLEAR_IRQ_STATUS_6X && n==2); irq &= ~b[1];
   // Next frame begins/completes while the previous FIFO is being serviced.
   irq |= nextIrq; packetAvailable = nextIrq & IRQ_RX_DONE_MASK_6X;
 }
 void readBuffer(uint8_t* b, int n) { memset(b,0xa5,n); }
};
''' + source[start:end] + r'''
int main() {
 static_assert(handheld::sx1262_timing::receivePreambleSymbols(18)==65);
 static_assert(handheld::sx1262_timing::receivePreambleSymbols(25)==65);
 static_assert(handheld::sx1262_timing::receivePreambleSymbols(100)==100);
 SX1262 r;
 r.nextIrq=IRQ_PREAMBLE_DET_MASK_6X;
 assert(r.parsePacket()==17); r.resumeReceive();
 assert(r.irq==IRQ_PREAMBLE_DET_MASK_6X && r.restarts==0 && r.reads==1);
 r.irq=IRQ_RX_DONE_MASK_6X; r.nextIrq=IRQ_RX_DONE_MASK_6X;
 assert(r.parsePacket()==17); r.resumeReceive();
 assert(r.packetAvailable && r.irq==IRQ_RX_DONE_MASK_6X && r.restarts==0);
 // CRC belongs to the original snapshot, even if a valid next packet arrives.
 r.irq=IRQ_RX_DONE_MASK_6X | IRQ_PAYLOAD_CRC_ERROR_MASK_6X;
 assert(r.parsePacket()==0); assert(r.packetAvailable && r.restarts==0);
 r.nextIrq=0; assert(r.parsePacket()==17); assert(r.irq==0);
 // No completed packet must neither acknowledge nor restart the next preamble.
 r.irq=IRQ_PREAMBLE_DET_MASK_6X;
 assert(r.parsePacket()==0); r.resumeReceive(); assert(r.irq==4 && r.restarts==0);
 // Restore RX if hardware left it; don't interfere with active TX or failed I/O.
 r.status=0x32; r.resumeReceive(); assert(r.restarts==1);
 r.status=0x62; r._txActive=true; r.resumeReceive(); assert(r.restarts==1);
 r._txActive=false; r._ioFailed=true; r.resumeReceive(); assert(r.restarts==1);
 SX1262 busy; busy.irq=4; clock_ms=0;
 assert(busy.isRxBusy()); clock_ms=349; assert(busy.isRxBusy());
 clock_ms=350; assert(!busy.isRxBusy()); assert(busy.restarts==0);
 busy.irq=16; assert(busy.isRxBusy()); clock_ms=600; assert(busy.isRxBusy());
 busy.irq=2; assert(busy.isRxBusy()); // completed frame cannot be overwritten
 busy.irq=0; assert(!busy.isRxBusy());
 clock_ms=0xfffffff0; busy.irq=4; assert(busy.isRxBusy());
 clock_ms=50; assert(busy.isRxBusy()); clock_ms=350; assert(!busy.isRxBusy());
 // Header arriving while an expired preamble is cleared keeps TX deferred.
 busy.irq=4; clock_ms=1000; assert(busy.isRxBusy());
 busy.nextIrq=16; clock_ms=1400; assert(busy.isRxBusy());
 busy.nextIrq=0; clock_ms=20000; assert(busy.isRxBusy());
 clock_ms=30000; assert(!busy.isRxBusy());
 assert(busy.restarts==0);

}
'''
with tempfile.TemporaryDirectory(prefix='standalone-rx-') as directory:
    cpp=Path(directory)/'test.cpp'; binary=Path(directory)/'test'
    cpp.write_text(harness)
    subprocess.run(['c++','-std=c++17','-Wall','-Wextra','-Werror','-Wno-unused-parameter',
                    '-I',str(ROOT/'src/core/radio'),str(cpp),'-o',str(binary)],check=True)
    subprocess.run([str(binary)],check=True)
print('Standalone RX: next preamble/packet preserved, CRC snapshot, idle and recovery passed')
