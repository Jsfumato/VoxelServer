CREATE DATABASE voxelData;

use voxelData;

# for login Data
CREATE TABLE IF NOT EXISTS loginData
(
	email VARCHAR(50) NOT NULL UNIQUE,
    `password` VARCHAR(20) NOT NULL,
    uID CHAR(10) UNIQUE,
    
    CONSTRAINT pk_email primary key (email)
);
DELETE FROM loginData;
SELECT * FROM loginData;

# for user data
Create Table If NOT EXists playerData 
(
	uID CHAR(10) PRIMARY KEY,
    userName VARCHAR(10) 
);
INSERT into playerData values("aaaaaaaaaa", "dummy");
DELETE FROM playerData;
SELECT * FROM playerData;


# for characterData
Create Table If NOT EXists characterData 
(
	uUserID 		CHAR(10) NOT NULL,
    uCharacterID 	CHAR(10) UNIQUE NOT NULL,
    characterName 	VARCHAR(10) NOT NULL,
    `level`			int NOT NULL,
    
    CONSTRAINT pk_userCharacterID PRIMARY KEY (uUserID, uCharacterID),
	CONSTRAINT fk_userID FOREIGN KEY (uUserID) references playerData(uID)
);

INSERT INTO characterData values("aaaaaaaaaa", "c000000000", "dumdum", 1);
SELECT * FROM characterData;


# for dungeon Data
Drop Table If Exists dungeonData;
Create Table IF NOT EXISTS dungeonData
(
	uUserID 		CHAR(10) NOT NULL,
    uDungeonID 		CHAR(10) UNIQUE NOT NULL,
    dungeonName 	VARCHAR(10) NOT NULL,
    info 			VARCHAR(30),
    hasRoomNum		int NOT NULL DEFAULT 1,
    
    CONSTRAINT pk_userDungeonID PRIMARY KEY (uUserID, uDungeonID),
    CONSTRAINT fk_dUserID FOREIGN KEY (uUserID) references playerData(uID)
);
INSERT INTO dungeonData values("aaaaaaaaaa", "d000000000", "dundummy", "test dummy dungeon", 1);


# for Block Data
Drop Table If exists blockDictionary;
CREATE TABLE IF NOt EXISTS blockDictionary
(
	uBlockID 		int NOT NULL,
    blockName		VARCHAR(20) NOT NULL,
    
    CONSTRAINT pk_blockID PRIMARY KEY (uBlockID)
);
INSERT INTO blockDictionary values(2, "stone");

# for block pos Data
DROP TABLE IF EXISTS mapData;
Create Table If NOT EXists mapData 
(
	uUserID 		CHAR(10) NOT NULL,
    uDungeonID 		CHAR(10) NOT NULL,
    roomNum			int NOT NULL,
    uBlockID		int NOT NULL,
    posX			int NOT NULL,
    posY			int NOT NULL,
    posZ			int NOT NULL,
    rotate			DECIMAL(4, 1) NOT NULL,
    
    CONSTRAINT pk_userDunID PRIMARY KEY (uUserID, uDungeonID, posX, posY, posZ),
    
    CONSTRAINT fk_mUserID FOREIGN KEY (uUserID) references playerData(uID),
    CONSTRAINT fk_mDungeonID FOREIGN KEY (uDungeonID) references dungeonData(uDungeonID),
    CONSTRAINT fk_mBlockID FOREIGN KEY (uBlockID) references blockDictionary(uBlockID)
);


# create user
drop USER 'voxel'@'%';
drop USER 'voxel'@'localhost';

CREATE USER 'voxel'@'%' identified BY '!!@@##$$';
CREATE USER 'voxel'@'localhost' identified BY '!!@@##$$';

GRANT ALL PRIVILEGES ON voxelData.* TO 'voxel'@'%';
GRANT ALL PRIVILEGES ON voxelData.* TO 'voxel'@'localhost';

DELETE FROM dungeonData WHERE uDungeonID != "d000000000";
DELETE FROM dungeonData;
SELECT * FROM dungeonData;
call SaveDungeonNameInfo("aaaaaaaaaa", "testDun", "INFOINFOINFO");

SELECT * FROM mapData;
DELETE FROM mapData;