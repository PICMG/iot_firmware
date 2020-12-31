//*******************************************************************
//    pldm.h
//
//    This file provides definitions of common pldm structures and 
//    numeric codes. This header is intended to be used as part of 
//    the PICMG pldm library reference code. 
//    
//    Portions of this code are based on the Platform Level Data Model
//    (PLDM) specifications from the Distributed Management Task Force 
//    (DMTF).  More information about PLDM can be found on the DMTF
//    web site (www.dmtf.org).
//
//    More information on the PICMG IoT data model can be found within
//    the PICMG family of IoT specifications.  For more information,
//    please visit the PICMG web site (www.picmg.org)
//
//    Copyright (C) 2020,  PICMG
//
//    This program is free software: you can redistribute it and/or modify
//    it under the terms of the GNU General Public License as published by
//    the Free Software Foundation, either version 3 of the License, or
//    (at your option) any later version.
//
//    This program is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//    GNU General Public License for more details.
//
//    You should have received a copy of the GNU General Public License
//    along with this program.  If not, see <https://www.gnu.org/licenses/>.
//
#pragma once
#ifdef __cplusplus
    #include <cstdint>
#else
    #include <stdint.h>
#endif

// data types
typedef uint16_t uint16;
typedef int16_t  sint16;
typedef uint8_t  enum8;
typedef uint8_t  bool8;
typedef uint8_t  uint8;
typedef uint8_t  bitfield8;
typedef int8_t    sint8;
typedef float          real32;
typedef int32_t        sint32;
typedef uint32_t       uint32;
typedef struct {
    uint8_t bytes[13];
} timestamp104;

// common data values
#define PDR_TYPE_TERMINUS_LOCATOR                1
#define PDR_TYPE_NUMERIC_SENSOR                  2
#define PDR_TYPE_NUMERIC_SENSOR_INITIALIZATION   3
#define PDR_TYPE_STATE_SENSOR                    4
#define PDR_TYPE_STATE_SENSOR_INITIALIZATION     5
#define PDR_TYPE_OEM_STATE_SET                   8
#define PDR_TYPE_NUMERIC_EFFECTER                9
#define PDR_TYPE_NUMERIC_EFFECTER_INITIALIZATION 10
#define PDR_TYPE_STATE_EFFECTER                  11
#define PDR_TYPE_STATE_EFFECTER_INITIALIZATION   12
#define PDR_TYPE_ENTITY_ASSOCIATION              15
#define PDR_TYPE_OEM_ENTITY_ID                   17
#define PDR_TYPE_FRU_RECORD_SET                  20

#define CMD_SET_TID                         0x01 // SetTID
#define CMD_GET_TID                         0x02 // GetTID
#define CMD_GET_TERMINUS_UID                0x03 // GetTerminusUID
#define CMD_SET_EVENT_RECEIVER              0x04 // SetEventReceiver
#define CMD_GET_EVENT_RECEIVER              0x05 // GetEventReceiver
#define CMD_PLATFORM_EVENT_MESSAGE          0x0A // PlatformEventMessage
#define CMD_POLL_FOR_PLATFORM_EVENT_MESSAGE 0x0B // PollForPlatformEventMessage
#define CMD_EVENT_MESSAGE_SUPPORTED         0x0C // EventMessageSupported
#define CMD_EVENT_MESSAGE_BUFFER_SIZE       0x0D // EventMessageBufferSize
#define CMD_SET_NUMERIC_SENSOR_ENABLE       0x10 // SetNumericSensorEnable
#define CMD_GET_SENSOR_READING              0x11 // GetSensorReading
#define CMD_GET_SENSOR_THRESHOLDS           0x12 // GetSensorThresholds
#define CMD_SET_SENSOR_THRESHOLDS           0x13 // SetSensorThresholds
#define CMD_RESTORE_SENSOR_THRESHOLDS       0x14 // RestoreSensorThresholds
#define CMD_GET_SENSOR_HYSTERESIS           0x15 // GetSensorHysteresis
#define CMD_SET_SENSOR_HYSTERESIS           0x16 // SetSensorHysteresis
#define CMD_INIT_NUMERIC_SENSOR             0x17 // InitNumericSensor
#define CMD_SET_STATE_SENSOR_ENABLES        0x20 // SetStateSensorEnables
#define CMD_GET_STATE_SENSOR_READINGS       0x21 // GetStateSensorReadings
#define CMD_INIT_STATE_SENSOR               0x22 // InitStateSensor
#define CMD_SET_NUMERIC_EFFECTER_ENABLE     0x30 // SetNumericEffecterEnable
#define CMD_SET_NUMERIC_EFFECTER_VALUE      0x31 // SetNumericEffecterValue
#define CMD_GET_NUMERIC_EFFECTER_VALUE      0x32 // GetNumericEffecterValue
#define CMD_SET_STATE_EFFECTER_ENABLES      0x38 // SetStateEffecterEnables
#define CMD_SET_STATE_EFFECTER_STATES       0x39 // SetStateEffecterStates
#define CMD_GET_STATE_EFFECTER_STATES       0x3A // GetStateEffecterStates
#define CMD_GET_PLDM_EVENT_LOG_INFO         0x40 // GetPLDMEventLogInfo
#define CMD_ENABLE_PLDM_EVENT_LOGGING       0x41 // EnablePLDMEventLogging
#define CMD_CLEAR_PLDM_EVENT_LOG            0x42 // ClearPLDMEventLog
#define CMD_GET_PLDM_EVENT_LOG_TIMESTAMP    0x43 // GetPLDMEventLogTimestamp
#define CMD_SET_PLDM_EVENT_LOG_TIMESTAMP    0x44 // SetPLDMEventLogTimestamp
#define CMD_READ_PLDM_EVENT_LOG             0x45 // ReadPLDMEventLog
#define CMD_GET_PLDM_EVENT_LOG_POLICY_INFO  0x46 // GetPLDMEventLogPolicyInfo
#define CMD_SET_PLDM_EVENT_LOG_POLICY       0x47 // SetPLDMEventLogPolicy
#define CMD_FIND_PLDM_EVENT_LOG_ENTRY       0x48 // FindPLDMEventLogEntry
#define CMD_GET_PDR_REPOSITORY_INFO         0x50 // GetPDRRepositoryInfo
#define CMD_GET_PDR                         0x51 // GetPDR
#define CMD_FIND_PDR                        0x52 // FindPDR
#define CMD_RUN_INIT_AGENT                  0x58 // RunInitAgent
#define CMD_GET_PDR_REPOSITORY_SIGNATURE    0x53 // GetPDRRepositorySignature

#define RESPONSE_SUCCESS                    0x00
#define RESPONSE_ERROR                      0x01
#define RESPONSE_ERROR_INVALID_DATA         0x02
#define RESPONSE_ERROR_INVALID_LENGTH       0x03
#define RESPONSE_ERROR_NOT_READY            0x04
#define RESPONSE_ERROR_UNSUPPORTED_PLDM_CMD 0x05
#define RESPONSE_ERROR_INVALID_PLDM_TYPE    0x20

#define RESPONSE_INVALID_PROTOCOL_TYPE              0x80
#define RESPONSE_INVALID_SENSOR_ID                  0x80
#define RESPONSE_INVALID_EFFECTER_ID                0x80
#define RESPONSE_INVALID_SEARCH_TYPE                0x80
#define RESPONSE_INVALID_DATA_TRANSFER_HANDLE       0x80
#define RESPONSE_INVALID_FIND_HANDLE                0x80
#define RESPONSE_ENABLE_METHOD_NOT_SUPPORTED        0x81
#define RESPONSE_UNSUPPORTED_EVENT_FORMAT_VERSION   0x81
#define RESPONSE_INVALID_SENSOR_OPERATIONAL_STATE   0x81
#define RESPONSE_REARM_UNAVAILABLE_IN_PRESENT_STATE 0x81
#define RESPONSE_UNSUPPORTED_SENSORSTATE            0x81
#define RESPONSE_INVALID_STATE_VALUE                0x81
#define RESPONSE_INVALID_TRANSFER_OPERATION_FLAG    0x81
#define RESPONSE_INVALID_FIND_OPERATION_FLAG        0x81
#define RESPONSE_HEARTBEAT_FREQUENCY_TOO_HIGH       0x82
#define RESPONSE_EVENT_ID_NOT_VALID                 0x82
#define RESPONSE_EVENT_GENERATION_NOT_SUPPORTED     0x82
#define RESPONSE_UNSUPPORTED_EFFECTERSTATE          0x82
#define RESPONSE_INVALID_ENTRY_ID                   0x82
#define RESPONSE_INVALID_RECORD_HANDLE              0x82
#define RESPONSE_INVALID_PDR_TYPE                   0x82
#define RESPONSE_INVALID_RECORD_CHANGE_NUMBER       0x83
#define RESPONSE_INVALID_PARAMETER_FORMAT_NUMBER    0x83
#define RESPONSE_TRANSFER_TIMEOUT                   0x84
#define RESPONSE_INVALID_FIND_PARAMETERS            0x84
#define RESPONSE_REPOSITORY_UPDATE_IN_PROGRESS      0x85

/*********************************************************
* Command and response structures
*/
#pragma pack(push)
#pragma pack(1)
typedef struct {
    unsigned char flags1;   // 7:rq, 6:D, 5:rsvd, 4:0: Instance Id
    unsigned char flags2;   // 7:6: Hdr Ver, 5:0: PldmType
    unsigned char command;
} PldmRequestHeader;

typedef struct {
    unsigned char flags1;   // 7:rq, 6:D, 5:rsvd, 4:0: Instance Id
    unsigned char flags2;   // 7:6: Hdr Ver, 5:0: PldmType
    unsigned char command;
    unsigned char completionCode;
} PldmResponseHeader;

typedef struct {
    enum8        completionCode;
    enum8        repositoryState;
    timestamp104 updateTime;
    timestamp104 OEMUpdateTime;
    uint32       recordCount;
    uint32       repositorySize;
    uint32       largestRecordSize;
    uint8        dataTransferHandleTimeout;
} GetPdrRepositoryInfoResponse;

typedef struct {
    uint32       recordHandle;
    uint32       dataTransferHandle;
    enum8        transferOperationFlag;
    uint16       requestCount;
    uint16       recordChangeNumber;
} GetPdrCommand;

typedef struct {
    enum8        completionCode;
    uint32       nextRecordHandle;
    uint32       nextDataTransferHandle;
    enum8        transferFlag;
    uint16       responseCount;
} GetPdrResponse;

/******************************************************************
* Platform Data Record Structures
*/
typedef struct {
    uint32 recordHandle;
    uint8 PDRHeaderVersion;
    uint8 PDRType;
    uint16 recordChangeNumber;
    uint16 dataLength;
} PdrCommonHeader;
#pragma pack(pop)

