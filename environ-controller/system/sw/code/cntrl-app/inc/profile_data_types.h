
#ifndef PROFILE_DATA_TYPES_H
#define PROFILE_DATA_TYPES_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif


/** @brief Invalid parameter constant */
#define			INVALID			-1
/** @brief Unknown parameter constant */
#define			UNKNOWN			-1

/**
 * @brief Modbus register for Boolean Type Profile variable
 * For storing YES/NO or TRUE/FALSE type information <br>
 * Profile/Tool variable with this type :
 * @ref Tools_Sensor_Fault <br>
 * @ref FXP1_OnOff <br>
 */
typedef 		uint16_t		mBOOL;
/**
 * @brief Modbus register for 32-bit integer Type Profile variable
 * For storing large time tick variables in ROM <br>
 * Variable with this type :
 * @ref POut_Pulse_Tmr_mSec <br>
 * Function with this type:
 * @ref time_to_sec
 */
typedef 		uint32_t 		mUINT;
/**
 * @brief Modbus register for 16-bit integer Type Profile variable
 * For storing integer ranging up to 65535
 * @ref Tools_Active_Profile
 * @ref Cir_Co2_Cyl_StPtPPM
 * @ref Cir_Co2_Gen_OffPPM
 */
typedef			uint16_t		mUSHORT;

/**
 * @brief Character type for storing strings like variables at EEPROM via modbus write
 * Combination of two characters are treated as a regiter
 * @ref Tools_User_Phone
 * @ref Tools_User_Email
 */
typedef 		uint8_t			mUCHAR;

/**
 * @brief Decisions constants
 */
typedef enum {
	NO = 0, /**< Decision constant NO = 0 */
	YES		/**< Decision constant YES = 1 */
}YN_e;

/**
 * @brief ON/OFF Status Constants
 */
typedef enum {
	OFF = 0,	/**< Decision constant, OFF = 0 */
	ON			/**< Decision constant, ON = 1 */
} OO_e;


/**
 * @brief Light Status Constants
 */
typedef enum {
	STATUS_DARK = 0,	/**< Cycle Status, DARK = 0 */
	STATUS_LIGHT,		/**< Cycle Status, LIGHT = 1 */
} LD_e;

/**
 * @brief BOOL constants
 **/
typedef enum {
	MBOOL_FALSE = 0, /**< Modbus Boolean variable FALSE */
	MBOOL_TRUE		 /**< Modbus Boolean variable TRUE */
} TF_e;

/**
 * @brief Open / Close Constants
 */
typedef enum {
	CLOSE = 0, /**< Constant CLOSE = 0 */
	OPEN       /**< Constant OPEN  = 1*/
}CO_e;


/**
 * @brief Cycle or Generation mode
 */
typedef enum {
	GEN = 0,	/**< Generator constant GEN = 0 */
	CYL,			/**< Cycle constant CYL = 1*/
	mCG_MAX = 0xFFFF
} mCG;

/**
 * @brief Light and Dark cycle definition
 */
typedef enum {
	DARK = 0,	/**< Dark cycle constant. DARK = 0 */
	LIGHT,		/**< Light Cycle constant LIGHT = 1 */
	mCLD_MAX = 0xFFFF
} mCLD;

/**
 * @brief Measurement Units
 */
typedef enum {
	UNIT_ENGLISH = 0, /**< Unit constant ENGLISH = 0 */
	UNIT_METRIC,		 /**< Unit constant METRIC = 1 */
	mUNIT_MAX = 0xFFFF
} mUNIT;

typedef enum {
    TWELEVE_HR = 0, /**< Unit constant ENGLISH = 0 */
    TWENTYFOUR_HR,         /**< Unit constant METRIC = 1 */
    mCLK_MAX = 0xFFFF
} mCLK;


#if 0
/**
 * @brief Light Operation Modes
 */
typedef enum {
	ALWAYS_OFF = 0, /**< Light Operation Cycle, Always OFF = 0 */
	ALWAYS_ON,		/**< Light Operation Cycle, Always ON = 1*/
	NORMAL,			/**< Light Operation Cycle, Normal = 2 */
	mOPRL_MAX = 0xFFFF
} mOPRL;
#endif

/**
 * @brief  System Operation Modes
 */
typedef enum {
	OPRS_NATIVE = 0, /**< NATIVE */
	OPRS_CIRC,		 /**< CIR (CIRC) */
	OPRS_FLEX,		/**< FLEX */
	OPRS_FOLLOW,	/**< FOLLOW */
	OPRS_FLIP,		/**< FLIP */
	mOPRS_MAX = 0xFFFF
} mOPRS;


/**
 * @brief FXP2 ON/OFF Modes
 */
typedef enum {
	OPRM_FF = 0,	/**< Operation Flip-Flop */
	OPRM_FLEX,		/**< Operation Flex */
	OPRM_FOLLOW,		/**< Operation Follow */
	mOPRM_MAX = 0xFFFF
} mOPRM;


/**
 * @brief Flex Operation Modes
 */
typedef enum {
	OPRF_NOP = 0,	/**< No Operation */
	OPRF_RPTTMR,	/**< Flex Operation mode : Repeat Timer */
	OPRF_HEAT,		/**< Flex Operation mode : Heater */
	OPRF_COOL,		/**< Flex Operation mode : Cooler */
	OPRF_HUM,		/**< Flex Operation mode : Humidifier */
	OPRF_DEHUM,		/**< Flex Operation mode : De-humidifier */
	OPRF_CO2,		/**< Flex Operation mode : Carbon-dioxide */
	mOPRF_MAX = 0xFFFF
} mOPRF;

/**
 * @brief Outlet Modes
 */
typedef enum {
	OLTM_NOP = 0, /**< No Operation */
	OLTM_CIRC,	  /**< Outlet Mode : Circ */
	OLTM_LIGHT,   /**< Outlet Mode : Light */
	OLTM_VENT,	  /**< Outlet Mode : Vent */
	OLTM_FXP1,	  /**< Outlet Mode : FXP1 */
	OLTM_FXP2,	  /**< Outlet Mode : FXP2 */
	mOLTM_MAX = 0xFFFF
} mOLTM;


/**
 * @brief Custom Time Variable
 */
typedef struct __attribute__((__packed__)) time_m {
	uint16_t HH; 	/** Hour */
	uint16_t MM;	/** Minute */
	uint16_t SS;	/** Second */
} mTIME;

/**
 * @brief Custom Time Variable with AM/PM
 */
typedef struct __attribute__((__packed__)) time {
	uint16_t hour;		/** Hour */
	uint16_t minute;	/** Minute */
	uint16_t second;	/** Second */
	uint16_t amPm;		/** AM/PM */
} TIME;


#define 	IS_M_TIME(T)		((T.HH <= 23) &&\
								 (T.MM <= 59) &&\
								 (T.SS <= 59))

//#define 	IS_VAL_TIME_MINUTE(M)	(M <= 59)
//#define 	IS_VAL_TIME_SECOND(S)	(S <= 59)
typedef enum
{
    GROW_CONTROLLER,          //outlets 6
    LIGHT_CONTROLLER,         //outlets 4
    FXP_CONTROLLER,           //outlets 2
    CO2_CONTROLLER,           //outlets 2
    IRRIGATION_CONTROLLER     //outlets 0
} CONTROLLER_TYPE;

typedef enum
{
    DEVICE_LIGHT,
    DEVICE_HOOD_VENT,
    DEVICE_CIRC,
    DEVICE_FXP1,
    DEVICE_FXP2,
    DEVICE_VENT,
    DEVICE_NONE
} DEVICE_TYPE;


typedef enum
{
    SUSPENDED,
    FORCED,
    USUAL
} DEVICE_STATE;


typedef enum
{
    ALWAYS_ON,
    ALWAYS_OFF,
    NORMAL
} LIGHT_CYCLE_STATE;

typedef enum
{
    NATIVE,
    FOLLOW,
    FLIP,
    FLEXI
} DEVICE_OP_STATE;


typedef enum
{
    RPTTIMER,
    HEATER,
    COOLER,
    HUMIDIFIER,
    DEHUMIDIFIER,
    CO2,
    LEAKMODE
} DEVICE_OP_MODE;


typedef struct __attribute__((__packed__)) mDATE {
    uint16_t DD;    /** Date */
    uint16_t MM;    /** Month */
    uint16_t YY;    /** year */
    uint16_t WD;
} mDATE;


#define GET_TIME_IN_SECOND(P1,P2) { P2 = P1.HH*3600 + P1.MM*60 + P1.SS ;}




typedef enum {
    ENGLISH, 
    KOREAN,
    JAPANESE,
    CHINESE,
    GERMAN,
    OTHERS
} mLANGUAGE;
   

typedef enum
{
    CYLINDER,
    GENERATOR
} CO2_SOURCE; 

#ifdef __cplusplus
}
#endif

#endif 
