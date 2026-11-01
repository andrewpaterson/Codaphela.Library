#ifndef __DURABLE_SYNC_H__
#define __DURABLE_SYNC_H__


//Whether durable files are synced to disk (fsync) when a commit ends.  Turning it off is only safe where losing data in a power failure does not matter, such as tests.
enum EDurableSync
{
	DS_Unknown,
	DS_No,
	DS_Yes
};


#endif // __DURABLE_SYNC_H__

