#pragma once
typedef unsigned short uint16;
typedef short          sint16;
typedef unsigned char  enum8;
typedef unsigned char  bool8;
typedef unsigned char  uint8;
typedef unsigned char  bitfield8;
typedef signed char    sint8;
typedef float          real32;
typedef long           sint32;
typedef unsigned long  uint32;
typedef struct {
    unsigned char bytes[13];
} timestamp104;


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
    enum8        test;
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


////////////////////////////////////////////////////////////////////
// Terminus Locator
////////////////////////////////////////////////////////////////////
typedef struct {
    uint16 PLDMTerminusHandle;
    enum8 validity;
    uint8 TID;
    uint16 containerID;
    enum8 terminusLocatorType;
    uint8 terminusLocatorValueSize;
    uint8 EID; 
} PdrTerminusLocatorMctp;

////////////////////////////////////////////////////////////////////
// numeric sensor pdrs
////////////////////////////////////////////////////////////////////
typedef struct {
    uint16 PLDMTerminusHandle;
    uint16 sensorID;
    uint16 entityType;
    uint16 entityInstanceNumber;
    uint16 containerID;
    enum8 sensorInit;
    bool8 sensorAuxiliaryNamesPDR;
    enum8 baseUnit;
    sint8 unitModifier;
    enum8 rateUnit;
    uint8 baseOEMUnitHandle;
    enum8 auxUnit;
    sint8 auxUnitModifier;
    enum8 auxrateUnit;
    enum8 rel;
    uint8 auxOEMUnitHandle;
    bool8 isLinear;
    enum8 sensorDataSize;
    real32 resolution;
    real32 offset;
    uint16 accuracy;
    uint8 plusTolerance;
    uint8 minusTolerance;
    sint32 hysteresis;
    bitfield8 supportedThresholds;
    bitfield8 thresholdAndHysteresisVolatility;
    real32 stateTransitionInterval;
    real32 updateInterval;
    sint32 maxReadable;
    sint32 minReadable;
    enum8 rangeFieldFormat;
    bitfield8 rangeFieldSupport;
    sint32 nominalValue;
    sint32 normalMax;
    sint32 normalMin;
    sint32 warningHigh;
    sint32 warningLow;
    sint32 criticalHigh;
    sint32 criticalLow;
    sint32 fatalHigh;
    sint32 fatalLow;
} PdrNumericSensorSint32;

typedef struct {
    uint16 PLDMTerminusHandle;
    uint16 sensorID;
    uint16 entityType;
    uint16 entityInstanceNumber;
    uint16 containerID;
    enum8 sensorInit;
    bool8 sensorAuxiliaryNamesPDR;
    enum8 baseUnit;
    sint8 unitModifier;
    enum8 rateUnit;
    uint8 baseOEMUnitHandle;
    enum8 auxUnit;
    sint8 auxUnitModifier;
    enum8 auxrateUnit;
    enum8 rel;
    uint8 auxOEMUnitHandle;
    bool8 isLinear;
    enum8 sensorDataSize;
    real32 resolution;
    real32 offset;
    uint16 accuracy;
    uint8 plusTolerance;
    uint8 minusTolerance;
    uint32 hysteresis;
    bitfield8 supportedThresholds;
    bitfield8 thresholdAndHysteresisVolatility;
    real32 stateTransitionInterval;
    real32 updateInterval;
    uint32 maxReadable;
    uint32 minReadable;
    enum8 rangeFieldFormat;
    bitfield8 rangeFieldSupport;
    uint32 nominalValue;
    uint32 normalMax;
    uint32 normalMin;
    uint32 warningHigh;
    uint32 warningLow;
    uint32 criticalHigh;
    uint32 criticalLow;
    uint32 fatalHigh;
    uint32 fatalLow;
} PdrNumericSensorUint32;

typedef struct {
    uint16 PLDMTerminusHandle;
    uint16 sensorID;
    uint16 entityType;
    uint16 entityInstanceNumber;
    uint16 containerID;
    enum8 sensorInit;
    bool8 sensorAuxiliaryNamesPDR;
    enum8 baseUnit;
    sint8 unitModifier;
    enum8 rateUnit;
    uint8 baseOEMUnitHandle;
    enum8 auxUnit;
    sint8 auxUnitModifier;
    enum8 auxrateUnit;
    enum8 rel;
    uint8 auxOEMUnitHandle;
    bool8 isLinear;
    enum8 sensorDataSize;
    real32 resolution;
    real32 offset;
    uint16 accuracy;
    uint8 plusTolerance;
    uint8 minusTolerance;
    sint16 hysteresis;
    bitfield8 supportedThresholds;
    bitfield8 thresholdAndHysteresisVolatility;
    real32 stateTransitionInterval;
    real32 updateInterval;
    sint16 maxReadable;
    sint16 minReadable;
    enum8 rangeFieldFormat;
    bitfield8 rangeFieldSupport;
    sint16 nominalValue;
    sint16 normalMax;
    sint16 normalMin;
    sint16 warningHigh;
    sint16 warningLow;
    sint16 criticalHigh;
    sint16 criticalLow;
    sint16 fatalHigh;
    sint16 fatalLow;
} PdrNumericSensorSint16;

typedef struct {
    uint16 PLDMTerminusHandle;
    uint16 sensorID;
    uint16 entityType;
    uint16 entityInstanceNumber;
    uint16 containerID;
    enum8 sensorInit;
    bool8 sensorAuxiliaryNamesPDR;
    enum8 baseUnit;
    sint8 unitModifier;
    enum8 rateUnit;
    uint8 baseOEMUnitHandle;
    enum8 auxUnit;
    sint8 auxUnitModifier;
    enum8 auxrateUnit;
    enum8 rel;
    uint8 auxOEMUnitHandle;
    bool8 isLinear;
    enum8 sensorDataSize;
    real32 resolution;
    real32 offset;
    uint16 accuracy;
    uint8 plusTolerance;
    uint8 minusTolerance;
    uint16 hysteresis;
    bitfield8 supportedThresholds;
    bitfield8 thresholdAndHysteresisVolatility;
    real32 stateTransitionInterval;
    real32 updateInterval;
    uint16 maxReadable;
    uint16 minReadable;
    enum8 rangeFieldFormat;
    bitfield8 rangeFieldSupport;
    uint16 nominalValue;
    uint16 normalMax;
    uint16 normalMin;
    uint16 warningHigh;
    uint16 warningLow;
    uint16 criticalHigh;
    uint16 criticalLow;
    uint16 fatalHigh;
    uint16 fatalLow;
} PdrNumericSensorUint16;

typedef struct {
    uint16 PLDMTerminusHandle;
    uint16 sensorID;
    uint16 entityType;
    uint16 entityInstanceNumber;
    uint16 containerID;
    enum8 sensorInit;
    bool8 sensorAuxiliaryNamesPDR;
    enum8 baseUnit;
    sint8 unitModifier;
    enum8 rateUnit;
    uint8 baseOEMUnitHandle;
    enum8 auxUnit;
    sint8 auxUnitModifier;
    enum8 auxrateUnit;
    enum8 rel;
    uint8 auxOEMUnitHandle;
    bool8 isLinear;
    enum8 sensorDataSize;
    real32 resolution;
    real32 offset;
    uint16 accuracy;
    uint8 plusTolerance;
    uint8 minusTolerance;
    sint8 hysteresis;
    bitfield8 supportedThresholds;
    bitfield8 thresholdAndHysteresisVolatility;
    real32 stateTransitionInterval;
    real32 updateInterval;
    sint8 maxReadable;
    sint8 minReadable;
    enum8 rangeFieldFormat;
    bitfield8 rangeFieldSupport;
    sint8 nominalValue;
    sint8 normalMax;
    sint8 normalMin;
    sint8 warningHigh;
    sint8 warningLow;
    sint8 criticalHigh;
    sint8 criticalLow;
    sint8 fatalHigh;
    sint8 fatalLow;
} PdrNumericSensorSint8;

typedef struct {
    uint16 PLDMTerminusHandle;
    uint16 sensorID;
    uint16 entityType;
    uint16 entityInstanceNumber;
    uint16 containerID;
    enum8 sensorInit;
    bool8 sensorAuxiliaryNamesPDR;
    enum8 baseUnit;
    sint8 unitModifier;
    enum8 rateUnit;
    uint8 baseOEMUnitHandle;
    enum8 auxUnit;
    sint8 auxUnitModifier;
    enum8 auxrateUnit;
    enum8 rel;
    uint8 auxOEMUnitHandle;
    bool8 isLinear;
    enum8 sensorDataSize;
    real32 resolution;
    real32 offset;
    uint16 accuracy;
    uint8 plusTolerance;
    uint8 minusTolerance;
    uint8 hysteresis;
    bitfield8 supportedThresholds;
    bitfield8 thresholdAndHysteresisVolatility;
    real32 stateTransitionInterval;
    real32 updateInterval;
    uint8 maxReadable;
    uint8 minReadable;
    enum8 rangeFieldFormat;
    bitfield8 rangeFieldSupport;
    uint8 nominalValue;
    uint8 normalMax;
    uint8 normalMin;
    uint8 warningHigh;
    uint8 warningLow;
    uint8 criticalHigh;
    uint8 criticalLow;
    uint8 fatalHigh;
    uint8 fatalLow;
} PdrNumericSensorUint8;

////////////////////////////////////////////////////////////////////
// state sensor pdrs
////////////////////////////////////////////////////////////////////
typedef struct {
    uint16 PLDMTerminusHandle;
    uint16 sensorID;
    uint16 entityType;
    uint16 entityInstanceNumber;
    uint16 containerID;
    enum8 sensorInit;
    bool8 sensorAuxiliaryNamesPDR;
    uint8 compositeSensorCount;
    uint16 stateSetID;
    uint8 possibleStatesSize;
    bitfield8 possibleStates;
} PdrStateSensorSingle;

////////////////////////////////////////////////////////////////////
// numeric effecter pdrs
////////////////////////////////////////////////////////////////////
typedef struct {
    uint16 PLDMTerminusHandle;
    uint16 effecterID;
    uint16 entityType;
    uint16 entityInstanceNumber;
    uint16 containerID;
    uint16 effecterSemanticID;
    enum8 effecterInit;
    bool8 effecterAuxiliaryNamesPDR;
    enum8 baseUnit;
    sint8 unitModifier;
    enum8 rateUnit;
    uint8 baseOEMUnitHandle;
    enum8 auxUnit;
    sint8 auxUnitModifier;
    enum8 auxrateUnit;
    uint8 auxOEMUnitHandle;
    bool8 isLinear;
    enum8 effecterDataSize;
    real32 resolution;
    real32 offset;
    uint16 accuracy;
    uint8 plusTolerance;
    uint8 minusTolerance;
    real32 stateTransitionInterval;
    real32 transitionInterval;
    sint32 maxSettable;
    sint32 minSettable;
    enum8 rangeFieldFormat;
    bitfield8 rangeFieldSupport;
    sint32 nominalValue;
    sint32 normalMax;
    sint32 normalMin;
    sint32 ratedMax;
    sint32 ratedMin;
} PdrNumericEffecterSint32;

typedef struct {
    uint16 PLDMTerminusHandle;
    uint16 effecterID;
    uint16 entityType;
    uint16 entityInstanceNumber;
    uint16 containerID;
    uint16 effecterSemanticID;
    enum8 effecterInit;
    bool8 effecterAuxiliaryNamesPDR;
    enum8 baseUnit;
    sint8 unitModifier;
    enum8 rateUnit;
    uint8 baseOEMUnitHandle;
    enum8 auxUnit;
    sint8 auxUnitModifier;
    enum8 auxrateUnit;
    uint8 auxOEMUnitHandle;
    bool8 isLinear;
    enum8 effecterDataSize;
    real32 resolution;
    real32 offset;
    uint16 accuracy;
    uint8 plusTolerance;
    uint8 minusTolerance;
    real32 stateTransitionInterval;
    real32 transitionInterval;
    uint32 maxSettable;
    uint32 minSettable;
    enum8 rangeFieldFormat;
    bitfield8 rangeFieldSupport;
    uint32 nominalValue;
    uint32 normalMax;
    uint32 normalMin;
    uint32 ratedMax;
    uint32 ratedMin;
} PdrNumericEffecterUint32;

typedef struct {
    uint16 PLDMTerminusHandle;
    uint16 effecterID;
    uint16 entityType;
    uint16 entityInstanceNumber;
    uint16 containerID;
    uint16 effecterSemanticID;
    enum8 effecterInit;
    bool8 effecterAuxiliaryNamesPDR;
    enum8 baseUnit;
    sint8 unitModifier;
    enum8 rateUnit;
    uint8 baseOEMUnitHandle;
    enum8 auxUnit;
    sint8 auxUnitModifier;
    enum8 auxrateUnit;
    uint8 auxOEMUnitHandle;
    bool8 isLinear;
    enum8 effecterDataSize;
    real32 resolution;
    real32 offset;
    uint16 accuracy;
    uint8 plusTolerance;
    uint8 minusTolerance;
    real32 stateTransitionInterval;
    real32 transitionInterval;
    sint16 maxSettable;
    sint16 minSettable;
    enum8 rangeFieldFormat;
    bitfield8 rangeFieldSupport;
    sint16 nominalValue;
    sint16 normalMax;
    sint16 normalMin;
    sint16 ratedMax;
    sint16 ratedMin;
} PdrNumericEffecterSint16;

typedef struct {
    uint16 PLDMTerminusHandle;
    uint16 effecterID;
    uint16 entityType;
    uint16 entityInstanceNumber;
    uint16 containerID;
    uint16 effecterSemanticID;
    enum8 effecterInit;
    bool8 effecterAuxiliaryNamesPDR;
    enum8 baseUnit;
    sint8 unitModifier;
    enum8 rateUnit;
    uint8 baseOEMUnitHandle;
    enum8 auxUnit;
    sint8 auxUnitModifier;
    enum8 auxrateUnit;
    uint8 auxOEMUnitHandle;
    bool8 isLinear;
    enum8 effecterDataSize;
    real32 resolution;
    real32 offset;
    uint16 accuracy;
    uint8 plusTolerance;
    uint8 minusTolerance;
    real32 stateTransitionInterval;
    real32 transitionInterval;
    uint16 maxSettable;
    uint16 minSettable;
    enum8 rangeFieldFormat;
    bitfield8 rangeFieldSupport;
    uint16 nominalValue;
    uint16 normalMax;
    uint16 normalMin;
    uint16 ratedMax;
    uint16 ratedMin;
} PdrNumericEffecterUint16;

typedef struct {
    uint16 PLDMTerminusHandle;
    uint16 effecterID;
    uint16 entityType;
    uint16 entityInstanceNumber;
    uint16 containerID;
    uint16 effecterSemanticID;
    enum8 effecterInit;
    bool8 effecterAuxiliaryNamesPDR;
    enum8 baseUnit;
    sint8 unitModifier;
    enum8 rateUnit;
    uint8 baseOEMUnitHandle;
    enum8 auxUnit;
    sint8 auxUnitModifier;
    enum8 auxrateUnit;
    uint8 auxOEMUnitHandle;
    bool8 isLinear;
    enum8 effecterDataSize;
    real32 resolution;
    real32 offset;
    uint16 accuracy;
    uint8 plusTolerance;
    uint8 minusTolerance;
    real32 stateTransitionInterval;
    real32 transitionInterval;
    sint8 maxSettable;
    sint8 minSettable;
    enum8 rangeFieldFormat;
    bitfield8 rangeFieldSupport;
    sint8 nominalValue;
    sint8 normalMax;
    sint8 normalMin;
    sint8 ratedMax;
    sint8 ratedMin;
} PdrNumericEffecterSint8;

typedef struct {
    uint16 PLDMTerminusHandle;
    uint16 effecterID;
    uint16 entityType;
    uint16 entityInstanceNumber;
    uint16 containerID;
    uint16 effecterSemanticID;
    enum8 effecterInit;
    bool8 effecterAuxiliaryNamesPDR;
    enum8 baseUnit;
    sint8 unitModifier;
    enum8 rateUnit;
    uint8 baseOEMUnitHandle;
    enum8 auxUnit;
    sint8 auxUnitModifier;
    enum8 auxrateUnit;
    uint8 auxOEMUnitHandle;
    bool8 isLinear;
    enum8 effecterDataSize;
    real32 resolution;
    real32 offset;
    uint16 accuracy;
    uint8 plusTolerance;
    uint8 minusTolerance;
    real32 stateTransitionInterval;
    real32 transitionInterval;
    uint8 maxSettable;
    uint8 minSettable;
    enum8 rangeFieldFormat;
    bitfield8 rangeFieldSupport;
    uint8 nominalValue;
    uint8 normalMax;
    uint8 normalMin;
    uint8 ratedMax;
    uint8 ratedMin;
} PdrNumericEffecterUint8;

/////////////////////////////////////////////////////////////////////////////////////////
// State Effecter PDR
/////////////////////////////////////////////////////////////////////////////////////////
typedef struct {
    uint16 PLDMTerminusHandle;
    uint16 effecterID;
    uint16 entityType;
    uint16 entityInstanceNumber;
    uint16 containerID;
    uint16 effecterSemanticId;
    enum8 effecterInit;
    bool8 effecterDescriptionPDR;
    uint8 compositeSensorCount;
    uint16 stateSetID;
    uint8 possibleStatesSize;
    bitfield8 possibleStates;
} PdrStateEffecterSingle;

/////////////////////////////////////////////////////////////////////////////////////////
// Oem State Set PDR
/////////////////////////////////////////////////////////////////////////////////////////
typedef struct {
    uint16 PLDMTerminusHandle;
    uint16 OEMStateSetIDHandle;
    uint32 vendorIANA;
    enum8  unspecifiedValueHint;
    uint8  stateCount;
} PdrOemStateSet;

/////////////////////////////////////////////////////////////////////////////////////////
// Oem State Value PDR
/////////////////////////////////////////////////////////////////////////////////////////
typedef struct {
    uint8  minStateValue;
    uint8  maxStateValue;
    uint8  stringCount;
    // ascii null terminated string language code
    // utf null terminated state name;
} OemStateValue;
#pragma pack(pop)

