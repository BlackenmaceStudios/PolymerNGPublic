// Snapshot.cpp
//

#include "pch.h"
#include "build.h"
#include "Snapshot.h"
#include "lz4.h"

extern "C" void initprintf(const char *f, ...);

void ReadPlayerSnapshotInfo(BitMsg &msg);
void WritePlayerSnapshotInfo(BitMsg &msg);
void NetModifyPlayerVisibility(SPRITETYPE *snapshotsprites);
void NetHideLocalPlayer();

Snapshot::Snapshot()
{
	memset(this, 0, sizeof(Snapshot));
}

bool CompareSprite(tspritetype &s1, tspritetype &s2)
{
	return (s1.x == s2.x && s1.y == s2.z && s1.z == s2.z &&
		s1.cstat == s2.cstat &&
		s1.picnum == s2.picnum &&
		s1.shade == s2.shade &&
		s1.pal == s2.pal && s1.clipdist == s2.clipdist && s1.blend == s2.blend &&
		s1.xrepeat == s2.xrepeat && s1.yrepeat == s2.yrepeat &&
		s1.xoffset == s2.xoffset && s1.yoffset == s2.yoffset &&
		s1.sectnum == s2.sectnum && s1.statnum == s2.statnum &&
		s1.ang == s2.ang && s1.owner == s2.owner && s1.xvel == s2.xvel && s1.yvel == s2.yvel && s1.zvel == s2.zvel &&
		s1.lotag == s2.lotag && s1.hitag == s2.hitag &&
		s1.extra == s2.extra);
}

bool CompareSector(sectortype &s1, sectortype &s2)
{
	if (s1.wallptr != s2.wallptr || s1.wallnum != s2.wallnum)
		return false;

	if (s1.ceilingz != s2.ceilingz || s1.floorz != s2.floorz)
		return false;

	if (s1.ceilingstat != s2.ceilingstat || s1.floorstat != s2.floorstat)
		return false;

	if (s1.ceilingpicnum != s2.ceilingpicnum || s1.ceilingheinum != s2.ceilingheinum)
		return false;

	if (s1.ceilingshade != s2.ceilingshade)
		return false;

	if (s1.ceilingpal != s2.ceilingpal || s1.ceilingxpanning != s2.ceilingxpanning || s1.ceilingypanning != s2.ceilingypanning)
		return false;

	if (s1.floorpicnum != s2.floorpicnum || s1.floorheinum != s2.floorheinum)
		return false;

	if (s1.floorshade != s2.floorshade)
		return false;

	if (s1.floorpal != s2.floorpal || s1.floorxpanning != s2.floorxpanning || s1.floorypanning != s2.floorypanning)
		return false;

	if (s1.lotag != s2.lotag || s1.hitag != s2.hitag)
		return false;

	if (s1.extra != s2.extra)
		return false;

	return true;
}

bool CompareWall(walltype &w1, walltype &w2)
{
	if (w1.x != w2.x || w1.y != w2.y)
		return false;

	if (w1.point2 != w2.point2 || w1.nextwall != w2.nextwall || w1.nextsector != w2.nextsector)
		return false;

	if (w1.cstat != w2.cstat)
		return false;

	if (w1.picnum != w2.picnum || w1.overpicnum != w2.overpicnum)
		return false;

	if (w1.shade != w2.shade)
		return false;

	if (w1.pal != w2.pal || w1.xrepeat != w2.xrepeat || w1.yrepeat != w2.yrepeat || w1.xpanning != w2.xpanning || w1.ypanning != w2.ypanning)
		return false;

	if (w1.lotag != w2.lotag || w1.hitag != w2.hitag)
		return false;

	if (w1.extra != w2.extra)
		return false;

	return true;
}

//
// CreateSnapshotPacket
//
void Snapshot::CreateSnapshotPacket(BitMsg &msg)
{
	static Snapshot *snapshot = NULL;
	static Snapshot *prevSnapShot = NULL;

	if (snapshot == NULL)
	{
		snapshot = new Snapshot();
		prevSnapShot = new Snapshot();

	//	snapshot->CreateSnapshot();
	//	memcpy(prevSnapShot, snapshot, sizeof(Snapshot));
	}
	snapshot->CreateSnapshot();

	NetModifyPlayerVisibility(&snapshot->_sprite[0]);

	int numWallsToUpdate = 0;
	int numSectorstoUpdate = 0;
	int numSpritesToUpdate = 0;
	{
		// Find how many walls have changed.

		for (int i = 0; i < MAXWALLS; i++)
		{
			if (CompareWall(snapshot->_wall[i], prevSnapShot->_wall[i]))
			{
				continue;
			}

			numWallsToUpdate++;
		}

		// Find how many sectors have changed.

		for (int i = 0; i < MAXSECTORS; i++)
		{
			if (CompareSector(snapshot->_sector[i], prevSnapShot->_sector[i]))
			{
				continue;
			}

			numSectorstoUpdate++;
		}

		// Find how many sprites have changed. 
		for (int i = 0; i < MAXSPRITES; i++)
		{
			if (CompareSprite(snapshot->_sprite[i], prevSnapShot->_sprite[i]) /*&& snapshot->spriteext[i] == snapshot->spriteext[i]*/)
			{
				continue;
			}
		
			numSpritesToUpdate++;
		}
	}

	msg.Write<int>(6666);

	// Write out the changed walls.
	msg.Write<int>(numWallsToUpdate);
	for (int i = 0; i < MAXWALLS; i++)
	{
		if (CompareWall(snapshot->_wall[i], prevSnapShot->_wall[i]))
		{
			continue;
		}
		
		msg.Write<int>(i);
		msg.WriteData((byte *)&snapshot->_wall[i], sizeof(walltype));
	}

	msg.Write<int>(6666);

	// Write out the changed sectors.
	msg.Write<int>(numSectorstoUpdate);
	for (int i = 0; i < MAXSECTORS; i++)
	{
		if (CompareSector(snapshot->_sector[i], prevSnapShot->_sector[i]))
		{
			continue;
		}

		msg.Write<int>(i);
		msg.WriteData((byte *)&snapshot->_sector[i], sizeof(sectortype));
	}

	msg.Write<int>(6666);

	// Write out the changed sprites
	msg.Write<int>(Numsprites);
	msg.Write<int>(numSpritesToUpdate);
	int numActualUpdatedSprites = 0;
	for (int i = 0; i < MAXSPRITES; i++)
	{
		if (CompareSprite(snapshot->_sprite[i], prevSnapShot->_sprite[i]) /*&& snapshot->spriteext[i] == snapshot->spriteext[i]*/)
		{
			continue;
		}
	
		msg.Write<int>(i);
		msg.WriteData((byte *)&snapshot->_sprite[i], sizeof(SPRITETYPE));

		numActualUpdatedSprites++;
	}

	msg.Write<int>(6666);

	//Write out the changed spritesext.
	//for (int i = 0; i < MAXSPRITES; i++)
	//{
	//	if (CompareSprite(snapshot->_sprite[i], prevSnapShot->_sprite[i]) /*&& snapshot->spriteext[i] == snapshot->spriteext[i]*/)
	//	{
	//		continue;
	//	}
	//
	//	msg.Write<int>(i);
	//	msg.WriteData((byte *)&snapshot->_spriteext[i], sizeof(spriteext_t));
	//}

	msg.WriteData((byte *)&snapshot->_snapshotSpriteStat.headspritesect[0], sizeof(int16_t) * MAXSECTORS);
	msg.WriteData((byte *)&snapshot->_snapshotSpriteStat.headspritestat[0], sizeof(int16_t) * MAXSTATUS);
	msg.WriteData((byte *)&snapshot->_snapshotSpriteStat.prevspritesect[0], sizeof(int16_t) * MAXSPRITES);
	msg.WriteData((byte *)&snapshot->_snapshotSpriteStat.prevspritestat[0], sizeof(int16_t) * MAXSPRITES);
	msg.WriteData((byte *)&snapshot->_snapshotSpriteStat.nextspritesect[0], sizeof(int16_t) * MAXSPRITES);
	msg.WriteData((byte *)&snapshot->_snapshotSpriteStat.nextspritestat[0], sizeof(int16_t) * MAXSPRITES);

	WritePlayerSnapshotInfo(msg);

	memcpy(prevSnapShot, snapshot, sizeof(Snapshot));
}

//
// Snapshot::RestoreSnapshotPacket
//
void Snapshot::RestoreSnapshotPacket(BitMsg &msg)
{
	if (msg.Read<int>() != 6666)
	{
		initprintf("packet error\n");
	}

	int numWallsToUpdate = msg.Read<int>();
	for (int i = 0; i < numWallsToUpdate; i++)
	{
		int wallNum = msg.Read<int>();
		if (wallNum == 2358)
		{
			static bool myTempBool = false;
			myTempBool = true;
		}
		memcpy(&::wall[wallNum], msg.ReadData(sizeof(walltype)), sizeof(walltype));
	}

	if (msg.Read<int>() != 6666)
	{
		initprintf("packet error\n");
	}

	int numSectorsToUpdate = msg.Read<int>();
	for (int i = 0; i < numSectorsToUpdate; i++)
	{
		int sectorNum = msg.Read<int>();
		memcpy(&::sector[sectorNum], msg.ReadData(sizeof(sectortype)), sizeof(sectortype));
	}

	if (msg.Read<int>() != 6666)
	{
		initprintf("packet error\n");
	}

	Numsprites = msg.Read<int>();
	int numSpritesToUpdate = msg.Read<int>();
	for (int i = 0; i < numSpritesToUpdate; i++)
	{
		int spriteNum = msg.Read<int>();
		memcpy(&::sprite[spriteNum], msg.ReadData(sizeof(SPRITETYPE)), sizeof(SPRITETYPE));
	}

	if (msg.Read<int>() != 6666)
	{
		initprintf("packet error\n");
	}
	
	//for (int i = 0; i < numSpritesToUpdate; i++)
	//{
	//	int spriteNum = msg.Read<int>();
	//	memcpy(&::spriteext[spriteNum], msg.ReadData(sizeof(spriteext_t)), sizeof(spriteext_t));
	//}

	msg.ReadData((byte *)&headspritesect[0], sizeof(int16_t) * MAXSECTORS);
	msg.ReadData((byte *)&headspritestat[0], sizeof(int16_t) * MAXSTATUS);
	msg.ReadData((byte *)&prevspritesect[0], sizeof(int16_t) * MAXSPRITES);
	msg.ReadData((byte *)&prevspritestat[0], sizeof(int16_t) * MAXSPRITES);
	msg.ReadData((byte *)&nextspritesect[0], sizeof(int16_t) * MAXSPRITES);
	msg.ReadData((byte *)&nextspritestat[0], sizeof(int16_t) * MAXSPRITES);

	ReadPlayerSnapshotInfo(msg);

	NetHideLocalPlayer();
}

//
// Snapshot::CreateSnapshot
//
void Snapshot::CreateSnapshot()
{
	this->_numwalls = ::numwalls;
	memcpy(&this->_wall[0], &::wall[0], sizeof(walltype) * MAXWALLS);
	this->_numsectors = ::numsectors;
	memcpy(&this->_sector[0], &::sector[0], sizeof(sectortype) * MAXSECTORS);
	memcpy(&this->_sprite[0], &::sprite[0], sizeof(SPRITETYPE) * MAXSPRITES);
	memcpy(&this->_spriteext[0], &::spriteext[0], sizeof(spriteext_t) * MAXSPRITES);
	memcpy(&this->_snapshotSpriteStat.headspritesect[0], &::headspritesect[0], sizeof(int16_t) * MAXSECTORS);
	memcpy(&this->_snapshotSpriteStat.headspritestat[0], &::headspritestat[0], sizeof(int16_t) * MAXSTATUS);
	memcpy(&this->_snapshotSpriteStat.prevspritesect[0], &::prevspritesect[0], sizeof(int16_t) * MAXSPRITES);
	memcpy(&this->_snapshotSpriteStat.prevspritestat[0], &::prevspritestat[0], sizeof(int16_t) * MAXSPRITES);
	memcpy(&this->_snapshotSpriteStat.nextspritesect[0], &::nextspritesect[0], sizeof(int16_t) * MAXSPRITES);
	memcpy(&this->_snapshotSpriteStat.nextspritestat[0], &::nextspritestat[0], sizeof(int16_t) * MAXSPRITES);
}

//
// RestoreSnapshot
//
void Snapshot::RestoreSnapshot()
{
	//::numwalls = numwalls;
	//memcpy(&::wall[0], &wall[0], sizeof(walltype) * MAXWALLS);
	//::numsectors = numsectors;
	//memcpy(&::sector[0], &sector[0], sizeof(sectortype) * MAXSECTORS);
	//memcpy(&::sprite[0], &sprite[0], sizeof(SPRITETYPE) * MAXSPRITES);
	//memcpy(&::spriteext[0], &spriteext[0], sizeof(spriteext_t) * MAXSPRITES);
	//memcpy(&::headspritesect[0], &snapshotSpriteStat.headspritesect[0], sizeof(int16_t) * MAXSECTORS);
	//memcpy(&::headspritestat[0], &snapshotSpriteStat.headspritestat[0], sizeof(int16_t) * MAXSTATUS);
	//memcpy(&::prevspritesect[0], &snapshotSpriteStat.prevspritesect[0], sizeof(int16_t) * MAXSPRITES);
	//memcpy(&::prevspritestat[0], &snapshotSpriteStat.prevspritestat[0], sizeof(int16_t) * MAXSPRITES);
	//memcpy(&::nextspritesect[0], &snapshotSpriteStat.nextspritesect[0], sizeof(int16_t) * MAXSPRITES);
	//memcpy(&::nextspritestat[0], &snapshotSpriteStat.nextspritestat[0], sizeof(int16_t) * MAXSPRITES);
}