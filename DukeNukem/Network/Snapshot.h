// Snapshot.h
//

#pragma once

#include "BitMsg.h"

//
// SnapshotSpriteStat
//
struct SnapshotSpriteStat
{
	int16_t headspritesect[MAXSECTORS + 1];
	int16_t headspritestat[MAXSTATUS + 1];
	int16_t prevspritesect[MAXSPRITES];
	int16_t prevspritestat[MAXSPRITES];
	int16_t nextspritesect[MAXSPRITES];
	int16_t nextspritestat[MAXSPRITES];
};

//
// Snapshot
//
class Snapshot
{
public:
	Snapshot();
	void				CreateSnapshot();
	void				RestoreSnapshot();

	static void			CreateSnapshotPacket(BitMsg &msg);
	static void			RestoreSnapshotPacket(BitMsg &msg);
public:
	int16_t _numwalls;
	walltype _wall[MAXWALLS];
	int16_t _numsectors;
	sectortype _sector[MAXSECTORS];
	SPRITETYPE _sprite[MAXSPRITES];
	spriteext_t _spriteext[MAXSPRITES];
	SnapshotSpriteStat _snapshotSpriteStat;
};