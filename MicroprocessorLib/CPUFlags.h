#ifndef __CPU_FLAGS_H__
#define __CPU_FLAGS_H__


#define STATUS_CARRY				0x01
#define STATUS_ZERO					0x02
#define STATUS_INTERRUPT_DISABLE	0x04
#define STATUS_DECIMAL				0x08

#define STATUS_BREAK				0X10	/* In emulation mode			*/
#define STATUS_INDEX_WIDTH			0X10	/* In native mode (x 0, 16 bit) */
#define STATUS_ACCUMULATOR_WIDTH	0X20

#define STATUS_OVERFLOW				0X40
#define STATUS_NEGATIVE				0X80


#endif // __CPU_FLAGS_H__

