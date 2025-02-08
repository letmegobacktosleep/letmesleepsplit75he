// Extern global variables
extern row_pins[ROWS_PER_HAND];
extern col_pins[MATRIX_COLS];
// Extern joystick definitions
#ifdef ANALOG_KEY_VIRTUAL_AXES
extern int8_t virtual_axes_from_self[6][4];
extern int8_t virtual_axes_from_slave[6][4];
# ifdef JOYSTICK_COORDINATES_ONE
extern const uint8_t joystick_coordinates_one[4][2];
# endif
# ifdef JOYSTICK_COORDINATES_TWO
extern const uint8_t joystick_coordinates_two[4][2];
# endif
# ifdef MOUSE_COORDINATES_ONE
extern const uint8_t mouse_coordinates_two[4][2];
# endif
# ifdef MOUSE_COORDINATES_TWO
extern const uint8_t mouse_coordinates_two[4][2];
# endif
# ifdef SCROLL_COORDINATES_ONE
extern const uint8_t scroll_coordinates_one[4][2];
# endif
# ifdef SCROLL_COORDINATES_TWO
extern const uint8_t scroll_coordinates_two[4][2];
# endif
#endif

typedef struct PACKED { 

    // All the settings
    uint8_t mode;   // actuation mode // 0 = normal // 2 = rapid trigger // 5,6,7,8 = DKS
    uint8_t lower;  // actuation point
    uint8_t upper;  // deadzone
    uint8_t down;   // rapid trigger sensitivity
    uint8_t up;     // rapid trigger sensitivity

} analog_config_t; // 5 bytes
_Static_assert(sizeof(analog_config_t)*MATRIX_ROWS*MATRIX_COLS == EECONFIG_USER_DATA_SIZE, "Mismatch in user EECONFIG stored data size");
extern analog_key_t analog_config[MATRIX_ROWS][MATRIX_COLS];

typedef struct {

    // Calibration settings
    uint16_t rest;  // analog value when key is at rest

    // Stuff that changes
    uint8_t mode;   // copy over mode from analog_config in matrix_init
    uint8_t old;    // old displacement, initialize to zero
    
} analog_key_t; // 4 bytes
extern analog_key_t analog_key[MATRIX_ROWS][MATRIX_COLS];

typedef struct PACKED {

    // Get displacement from gauss
    double lut_a;        // 8 bytes
    double lut_b;        // 8 bytes
    double lut_c;        // 8 bytes
    double lut_d;        // 8 bytes
    // Define the maximum values
    uint16_t max_input;  // 1 byte
    uint16_t max_output; // 1 byte

} lookup_table_t; // 36 bytes

typedef struct PACKED {

    lookup_table_t displacement; // 36 bytes
    lookup_table_t joystick;     // 36 bytes
    lookup_table_t multiplier;   // 36 bytes

} calibration_parameters_t; // 108 bytes
_Static_assert(sizeof(calibration_parameters_t) == EECONFIG_KB_DATA_SIZE, "Mismatch in keyboard EECONFIG stored data size");
extern calibration_parameters_t calibration_parameters;

// Function prototypes
void register_key(matrix_row_t *current_row, uint8_t current_col);
void deregister_key(matrix_row_t *current_row, uint8_t current_col);
void matrix_init_custom(void);
bool matrix_scan_custom(matrix_row_t current_matrix[]);