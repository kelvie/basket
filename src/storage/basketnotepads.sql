-- content table 
CREATE TABLE "content" (
    "content_id" INTEGER PRIMARY KEY, -- local id
    "note_id" INTEGER  NOT NULL CONSTRAINT fk_content2note REFERENCES "note"("note_id") ON DELETE CASCADE, -- must be linked to a note
    "path_or_text" TEXT NULL, -- if content_type is text then path_or_text contains text else datafile's path.
    "crypted" CHAR(1) NOT NULL DEFAULT 'N', -- indicate that basketnote are password crypted.
    "contenttype_id" INTEGER  NOT NULL CONSTRAINT fk_content2contenttype REFERENCES "contenttype"("contenttype_id") ON DELETE CASCADE, -- type of this content
    "content_uuid" CHAR(36) NOT NULL DEFAULT ('00000000-0000-0000-0000-000000000000') unique -- unique id used as references when export or share basketnote

);

-- contenttype table
CREATE TABLE "contenttype" (
    "contenttype_id" INTEGER PRIMARY KEY, -- local id
    "mimetype" TEXT NOT NULL,
    "contenttype_uuid" CHAR(36) NOT NULL DEFAULT ('00000000-0000-0000-0000-000000000000') unique -- unique id used as references when export or share basketnote
);

-- note table
CREATE TABLE "note" (
    "note_id" INTEGER PRIMARY KEY, -- local identity
    "basketnote_parent_id" INTEGER NOT NULL CONSTRAINT fk_note2basketnote REFERENCES "basketnote"("basketnote_id") ON DELETE CASCADE, -- must be linked to a basketnote
    "note_parent_id" INTEGER NULL CONSTRAINT fk_note2note REFERENCES "note"("note_id") ON DELETE CASCADE, -- can be linked to another note, if not then is a root note
    "note_uuid" CHAR(36) NOT NULL DEFAULT ('00000000-0000-0000-0000-000000000000') unique -- unique id used as references when export or share basketnote
);

-- basketnote table
CREATE TABLE "basketnote" (
"basketnote_id" INTEGER PRIMARY KEY, -- local id
"name" TEXT NOT NULL, -- local name for research
"crypted" CHAR(1) NOT NULL DEFAULT 'N',
"basketnote_uuid" CHAR(36) NOT NULL DEFAULT ('00000000-0000-0000-0000-000000000000') unique -- unique id used as references when export or share basketnote
);

--  metadata
CREATE TABLE "metadata" (
"metadata_id" INTEGER PRIMARY KEY, -- local id
"metadata_type_id" INTEGER NOT NULL CONSTRAINT fk_metadata_type REFERENCES "metadata_type"("metadata_type_id") ON DELETE CASCADE,
"metadata_uuid" CHAR(36) NOT NULL DEFAULT ('00000000-0000-0000-0000-000000000000') unique  -- unique id used as references when export or share basketnote
);

--  metadata_type
CREATE TABLE "metadata_type" (
"metadata_type_id" INTEGER PRIMARY KEY, -- local id
"name" TEXT NOT NULL unique, -- name for this type
"metadata_type_uuid" CHAR(36) NOT NULL DEFAULT ('00000000-0000-0000-0000-000000000000') unique -- unique id used as references when export or share basketnote
);

--  item_type
CREATE TABLE "item_type" (
"item_type_id" INTEGER PRIMARY KEY, -- local id
"name" TEXT NOT NULL unique, -- name for this type
"item_type_uuid" CHAR(36) NOT NULL DEFAULT ('00000000-0000-0000-0000-000000000000') unique -- unique id used as references when export or share basketnote
);

--  item_with_metadata
CREATE TABLE "item_with_metadata" (
"item_id" INTEGER NOT NULL, -- link to local id
"item_type_id" INTEGER NOT NULL CONSTRAINT fk_item_type REFERENCES "item_type"("item_type_id") ON DELETE CASCADE, 
"metadata_id" INTEGER NOT NULL CONSTRAINT fk_metadata REFERENCES "metadata"("metadata_id") ON DELETE CASCADE,
"item_parameter" TEXT NULL, -- store value linked to item and metadata
primary key("item_id","item_type_id","metadata_id") 
);

--  metadata_value
CREATE TABLE "metadata_value" (
"contenttype_id" INTEGER NOT NULL CONSTRAINT fk_metadata2contenttype REFERENCES "contenttype"("contenttype_id") ON DELETE CASCADE, 
"metadata_id" INTEGER NOT NULL CONSTRAINT fk_metadata2metadata REFERENCES "metadata"("metadata_id") ON DELETE CASCADE,
"metadata_value" TEXT NULL, -- store value linked to contenttype and metadata
primary key("contenttype_id","metadata_id") 
);
-- all basketnote example begin with an uuid 00000000-0000-0000-0000-xxxxxxxxxxxx

-- all item_type managed by basketnote core begin with an uuid 00000000-0000-0000-0000-xxxxxxxxxxxx
-- plugins must register their uuid to enforce integrity
INSERT INTO "item_type" VALUES(1,'basketnote','00000000-0000-0000-0000-000000000001');
INSERT INTO "item_type" VALUES(2,'note','00000000-0000-0000-0000-000000000002');
INSERT INTO "item_type" VALUES(3,'content','00000000-0000-0000-0000-000000000003');
INSERT INTO "item_type" VALUES(4,'contenttype','00000000-0000-0000-0000-000000000004');
INSERT INTO "item_type" VALUES(5,'metadata','00000000-0000-0000-0000-000000000005');
INSERT INTO "item_type" VALUES(6,'metadata_type','00000000-0000-0000-0000-000000000006');
INSERT INTO "item_type" VALUES(7,'item_type','00000000-0000-0000-0000-000000000007');
INSERT INTO "item_type" VALUES(8,'item_with_metadata','00000000-0000-0000-0000-000000000008');
INSERT INTO "item_type" VALUES(9,'metadata_value','00000000-0000-0000-0000-000000000009');


CREATE VIEW "vCryptedBasketNote" AS select basketnote_id, name from basketnote where crypted='Y';
-- INSERT INTO "item_type" VALUES(9,'metadata_value','00000000-0000-0000-0000-000000000009');
