/*
 Navicat Premium Dump SQL

 Source Server         : SmartCommunity
 Source Server Type    : SQLite
 Source Server Version : 3045000 (3.45.0)
 Source Schema         : main

 Target Server Type    : SQLite
 Target Server Version : 3045000 (3.45.0)
 File Encoding         : 65001

 Date: 18/07/2025 15:57:15
*/

PRAGMA foreign_keys = false;

-- ----------------------------
-- Table structure for _family_member_old_20250718
-- ----------------------------
DROP TABLE IF EXISTS "_family_member_old_20250718";
CREATE TABLE "_family_member_old_20250718" (
  "owner_id" INTEGER NOT NULL,
  "member_id" INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT,
  "relationship" TEXT NOT NULL,
  "phone" integer,
  "id_number" INTEGER NOT NULL,
  CONSTRAINT "fk_owner_id" FOREIGN KEY ("owner_id") REFERENCES "users" ("id") ON DELETE CASCADE ON UPDATE CASCADE
);

-- ----------------------------
-- Table structure for _property_info_old_20250718
-- ----------------------------
DROP TABLE IF EXISTS "_property_info_old_20250718";
CREATE TABLE "_property_info_old_20250718" (
  "property_id" INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT,
  "owner_id" INTEGER NOT NULL,
  "address" TEXT NOT NULL,
  "property_type" TEXT NOT NULL,
  "build_area" REAL,
  "property_status" text,
  "register_time" TEXT DEFAULT CURRENT_TIMESTAMP
);

-- ----------------------------
-- Table structure for announcement
-- ----------------------------
DROP TABLE IF EXISTS "announcement";
CREATE TABLE "announcement" (
  "id" INTEGER NOT NULL,
  "announcement_content" TEXT NOT NULL,
  "operator_id" INTEGER NOT NULL,
  "time" text NOT NULL,
  "title" TEXT NOT NULL,
  PRIMARY KEY ("id")
);

-- ----------------------------
-- Table structure for attendance
-- ----------------------------
DROP TABLE IF EXISTS "attendance";
CREATE TABLE "attendance" (
  "attendance_id" integer NOT NULL PRIMARY KEY AUTOINCREMENT,
  "staff_id" INTEGER NOT NULL,
  "attendance_date" TEXT NOT NULL,
  "check_in_time" TEXT,
  "check_out_time" TEXT,
  "status" integer NOT NULL DEFAULT 0
);

-- ----------------------------
-- Table structure for family_member
-- ----------------------------
DROP TABLE IF EXISTS "family_member";
CREATE TABLE "family_member" (
  "owner_id" INTEGER NOT NULL,
  "member_id" INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT,
  "relationship" TEXT NOT NULL,
  "phone" integer,
  "id_number" INTEGER NOT NULL,
  "name" TEXT NOT NULL,
  CONSTRAINT "fk_owner_id" FOREIGN KEY ("owner_id") REFERENCES "users" ("id") ON DELETE CASCADE ON UPDATE CASCADE
);

-- ----------------------------
-- Table structure for fee_log
-- ----------------------------
DROP TABLE IF EXISTS "fee_log";
CREATE TABLE "fee_log" (
  "fee_id" INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT,
  "owner_id" INTEGER NOT NULL,
  "fee_type" integer NOT NULL,
  "value" real NOT NULL,
  "payment_status" real NOT NULL,
  "time" DATE NOT NULL
);

-- ----------------------------
-- Table structure for item_fee
-- ----------------------------
DROP TABLE IF EXISTS "item_fee";
CREATE TABLE "item_fee" (
  "item_id" INTEGER NOT NULL,
  "fee" real NOT NULL,
  "unit" TEXT NOT NULL,
  PRIMARY KEY ("item_id")
);

-- ----------------------------
-- Table structure for leave_applications
-- ----------------------------
DROP TABLE IF EXISTS "leave_applications";
CREATE TABLE "leave_applications" (
  "app_id" INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT,
  "applicant_id" INTEGER NOT NULL,
  "start_date" TEXT NOT NULL,
  "end_date" TEXT NOT NULL,
  "reason" TEXT NOT NULL,
  "apply_time" TEXT NOT NULL,
  "approval_status" integer NOT NULL DEFAULT 0,
  "approver_id" INTEGER,
  "approve_time" text
);

-- ----------------------------
-- Table structure for owner_info
-- ----------------------------
DROP TABLE IF EXISTS "owner_info";
CREATE TABLE "owner_info" (
  "owner_id" INTEGER PRIMARY KEY AUTOINCREMENT,
  "name" TEXT NOT NULL,
  "id_number" TEXT,
  "phone_number" TEXT,
  "email" TEXT,
  "move_in_date" TEXT,
  "register_time" TEXT DEFAULT CURRENT_TIMESTAMP,
  UNIQUE ("id_number" ASC),
  UNIQUE ("phone_number" ASC)
);

-- ----------------------------
-- Table structure for parking_apply
-- ----------------------------
DROP TABLE IF EXISTS "parking_apply";
CREATE TABLE "parking_apply" (
  "apply_id" INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT,
  "owner_id" INTEGER NOT NULL,
  "plate" TEXT NOT NULL,
  "type" TEXT NOT NULL,
  "is_chargeable" integer NOT NULL,
  "size" text NOT NULL,
  "length" integer NOT NULL,
  "apply_time" DATE NOT NULL,
  "status" integer NOT NULL DEFAULT 0
);

-- ----------------------------
-- Table structure for parking_rental
-- ----------------------------
DROP TABLE IF EXISTS "parking_rental";
CREATE TABLE "parking_rental" (
  "rental_id" integer NOT NULL PRIMARY KEY AUTOINCREMENT,
  "space_id" INTEGER NOT NULL,
  "owner_id" INTEGER NOT NULL,
  "start_time" DATE NOT NULL,
  "end_time" DATE NOT NULL,
  "rent_fee" real NOT NULL,
  "payment_status" integer NOT NULL DEFAULT 0,
  "plate" TEXT NOT NULL
);

-- ----------------------------
-- Table structure for parking_space
-- ----------------------------
DROP TABLE IF EXISTS "parking_space";
CREATE TABLE "parking_space" (
  "space_id" INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT,
  "location" TEXT NOT NULL,
  "type" TEXT NOT NULL,
  "is_chargeable" integer NOT NULL,
  "size" text NOT NULL,
  "status" integer NOT NULL DEFAULT 0
);

-- ----------------------------
-- Table structure for property_info
-- ----------------------------
DROP TABLE IF EXISTS "property_info";
CREATE TABLE "property_info" (
  "property_id" INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT,
  "owner_id" INTEGER NOT NULL,
  "address" TEXT NOT NULL,
  "property_type" TEXT NOT NULL,
  "build_area" REAL NOT NULL,
  "property_status" text NOT NULL,
  "register_time" TEXT NOT NULL DEFAULT ''
);

-- ----------------------------
-- Table structure for repair_evaluation
-- ----------------------------
DROP TABLE IF EXISTS "repair_evaluation";
CREATE TABLE "repair_evaluation" (
  "order_id" INTEGER NOT NULL,
  "owner_id" INTEGER NOT NULL,
  "staff_id" INTEGER NOT NULL,
  "score" integer NOT NULL,
  "eval_content" TEXT NOT NULL,
  "eval_time" DATE NOT NULL,
  CONSTRAINT "fk_order_id" FOREIGN KEY ("order_id") REFERENCES "repair_order" ("order_id") ON DELETE CASCADE ON UPDATE CASCADE,
  CONSTRAINT "fk_owner_id" FOREIGN KEY ("owner_id") REFERENCES "users" ("id") ON DELETE NO ACTION ON UPDATE CASCADE,
  CONSTRAINT "fk_stuff_id" FOREIGN KEY ("staff_id") REFERENCES "users" ("id") ON DELETE NO ACTION ON UPDATE CASCADE,
  CONSTRAINT "ck_socre" CHECK ([score]>=0 AND [score]<=10)
);

-- ----------------------------
-- Table structure for repair_order
-- ----------------------------
DROP TABLE IF EXISTS "repair_order";
CREATE TABLE "repair_order" (
  "order_id" INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT,
  "owner_id" INTEGER NOT NULL,
  "problem_desc" TEXT NOT NULL,
  "order_time" text NOT NULL,
  CONSTRAINT "fk_owner_id" FOREIGN KEY ("owner_id") REFERENCES "users" ("id") ON DELETE CASCADE ON UPDATE CASCADE
);

-- ----------------------------
-- Table structure for repair_progress
-- ----------------------------
DROP TABLE IF EXISTS "repair_progress";
CREATE TABLE "repair_progress" (
  "order_id" INTEGER NOT NULL,
  "process_time" DATE NOT NULL,
  "process_desc" TEXT NOT NULL,
  "staff_id" INTEGER NOT NULL,
  "process_status" TEXT NOT NULL,
  CONSTRAINT "fk_order_id" FOREIGN KEY ("order_id") REFERENCES "repair_order" ("order_id") ON DELETE CASCADE ON UPDATE CASCADE,
  CONSTRAINT "fk_staff_id" FOREIGN KEY ("staff_id") REFERENCES "users" ("id") ON DELETE SET NULL ON UPDATE CASCADE
);

-- ----------------------------
-- Table structure for sqlite_sequence
-- ----------------------------
DROP TABLE IF EXISTS "sqlite_sequence";
CREATE TABLE "sqlite_sequence" (
  "name",
  "seq"
);

-- ----------------------------
-- Table structure for users
-- ----------------------------
DROP TABLE IF EXISTS "users";
CREATE TABLE "users" (
  "id" INTEGER NOT NULL COLLATE BINARY PRIMARY KEY AUTOINCREMENT,
  "username" TEXT NOT NULL COLLATE RTRIM,
  "password" text NOT NULL COLLATE RTRIM,
  "role" INTEGER NOT NULL,
  CONSTRAINT "username" UNIQUE ("username" COLLATE RTRIM ASC)
);

-- ----------------------------
-- Table structure for vehicle_access_log
-- ----------------------------
DROP TABLE IF EXISTS "vehicle_access_log";
CREATE TABLE "vehicle_access_log" (
  "log_id" INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT,
  "time" DATE NOT NULL,
  "type" integer NOT NULL,
  "plate" TEXT NOT NULL
);

-- ----------------------------
-- Auto increment value for _property_info_old_20250718
-- ----------------------------

-- ----------------------------
-- Auto increment value for attendance
-- ----------------------------
UPDATE "sqlite_sequence" SET seq = 2 WHERE name = 'attendance';

-- ----------------------------
-- Auto increment value for family_member
-- ----------------------------
UPDATE "sqlite_sequence" SET seq = 1 WHERE name = 'family_member';

-- ----------------------------
-- Auto increment value for fee_log
-- ----------------------------
UPDATE "sqlite_sequence" SET seq = 2 WHERE name = 'fee_log';

-- ----------------------------
-- Auto increment value for leave_applications
-- ----------------------------

-- ----------------------------
-- Auto increment value for owner_info
-- ----------------------------
UPDATE "sqlite_sequence" SET seq = 4 WHERE name = 'owner_info';

-- ----------------------------
-- Auto increment value for parking_apply
-- ----------------------------
UPDATE "sqlite_sequence" SET seq = 2 WHERE name = 'parking_apply';

-- ----------------------------
-- Auto increment value for parking_rental
-- ----------------------------
UPDATE "sqlite_sequence" SET seq = 2 WHERE name = 'parking_rental';

-- ----------------------------
-- Auto increment value for parking_space
-- ----------------------------
UPDATE "sqlite_sequence" SET seq = 3 WHERE name = 'parking_space';

-- ----------------------------
-- Auto increment value for property_info
-- ----------------------------
UPDATE "sqlite_sequence" SET seq = 1 WHERE name = 'property_info';

-- ----------------------------
-- Auto increment value for repair_order
-- ----------------------------
UPDATE "sqlite_sequence" SET seq = 1 WHERE name = 'repair_order';

-- ----------------------------
-- Auto increment value for users
-- ----------------------------
UPDATE "sqlite_sequence" SET seq = 8 WHERE name = 'users';

-- ----------------------------
-- Indexes structure for table users
-- ----------------------------
CREATE INDEX "idx_id"
ON "users" (
  "id" COLLATE BINARY ASC
);
CREATE UNIQUE INDEX "idx_username"
ON "users" (
  "username" COLLATE RTRIM ASC
);

-- ----------------------------
-- Auto increment value for vehicle_access_log
-- ----------------------------
UPDATE "sqlite_sequence" SET seq = 2 WHERE name = 'vehicle_access_log';

PRAGMA foreign_keys = true;
