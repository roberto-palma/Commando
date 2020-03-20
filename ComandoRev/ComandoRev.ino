#include <VarSpeedServo.h>

#define N_SERVOS 6
#define N_PARAMETERS 5
#define MAX_PARAMETER_LENGTH    12
#define MAX_COMMAND_LENGTH      36


struct servo_data_t {
    VarSpeedServo *servo = NULL;    // El servo al que cada instancia de la estructura va a manejar
    char name = 'h';
    int position = -1;
    int speed = -1;
    int min_pos = -1;
    int max_pos = -1;
};
typedef struct servo_data_t ServoData;

const int servo_pins[] = {3, 5, 6, 9, 10, 11};  // Los pines a los que los servos van conectados, el orden importa.
VarSpeedServo servos[N_SERVOS];                 // Los objetos de tipo VarSpeedServo que se encargaran de controlar
                                                // cada servo individualmente

ServoData servos_data[N_SERVOS];        // N instancias de la estructura para los N servos que van a manejar contienen 
                                        // un puntero al servo que cada instancia va a manejar

ServoData* selected = NULL;             // Este puntero va a apuntar a la informacion elegido a traves de la consola
                                        // Sera nulo si no se encuentra un servo con el nombre proporcionado

char command[MAX_COMMAND_LENGTH];       // Buffer para el comando recibido a traves de la consola
unsigned int command_length = 0;        // Guarda que tan largo fue el comando recibido
char parameters[N_PARAMETERS][MAX_PARAMETER_LENGTH];    // Guarda los parametros individuales para su uso mas adelante

void setup() {
    Serial.begin(9600);

    for(int i = 0; i < N_SERVOS; i++) {
        servos_data[i].servo = &servos[i];          // Le damos la direccion al objeto de tipo VarSpeedServo
                                                    // Correspondiente a su posicion en el arreglo
        servos_data[i].name = 97 + i;               // Les damos un caracter de la tabla ASCII 97 = a, 98 = b, etc...
        
        servos_data[i].servo->attach(servo_pins[i]);// Los servos_data se encuentran dentro de las estructuras asi que 
                                                    // se tienen que acceder a traves de la estructura (de preferencia)
    }
}

void loop() {
    if(Serial.available()) {
        ReceiveOrder();
        ServoMove();
        Serial.println();
    }
}

// Solo imprime los datos del servo actual
void PrintSelected() {
    Serial.print("Name: ");
    Serial.println(selected->name);
    Serial.print("Position: ");
    Serial.println(selected->position);
    Serial.print("Speed: ");
    Serial.println(selected->speed);
    
}

// Inyecta los valores obtenidos desde el serial a la estructura seleccionada
void InjectValuesToServo() {
    if(selected != NULL) {        

        selected->position = atoi(parameters[1]);
        selected->speed = atoi(parameters[2]);

        PrintSelected();
    }

}

// Selecciona una estructura en base al primer parametro que se nos dio en el serial
void SelectServo() {
    selected = NULL;
    for(int i = 0; i < N_SERVOS; i++) {
        if(parameters[0][0] == servos_data[i].name) {   // El primer caracter del primer parametro es el nombre
            selected = &servos_data[i];
        }
    }
}

// Divide la cadena recibida desde consola en parametros. El separador es una coma.
void SplitIntoParameters() {
    char* tokens = strtok(command, ",");
    int i = 0;
    while(tokens != NULL && i < MAX_PARAMETER_LENGTH) {
        memset(parameters[i], 0, MAX_PARAMETER_LENGTH);
        strcpy(parameters[i], tokens);
        tokens = strtok(NULL, ",");
        i++;
    }
}

// Escucha por informacion en el serial y en caso de recibir algo, lo procesa.
void ReceiveOrder() {
    if(Serial.available()) {
        memset(command, 0, MAX_COMMAND_LENGTH);
        command_length =  Serial.readBytesUntil('\n', command, MAX_COMMAND_LENGTH - 1);
        
        // Partimos el string en multiples parametros 
        SplitIntoParameters();

        // Seleccionamos un servo de acuerdo al primer parametro
        SelectServo();
        
        // Se inyectan los valores a la variable seleccionada si es que es valida
        InjectValuesToServo();
    }
}

// Mueve el servo
void ServoMove(){

    if(selected != NULL) {
        Serial.print("Done Servo ");
        Serial.println(selected->name);
        selected->servo->write(selected->position, selected->speed);
        delay(500);
    } else {
        Serial.print("Servo ");
        Serial.print(parameters[0]);
        Serial.println(" does not exist");
    }

}