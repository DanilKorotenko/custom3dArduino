#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define POINT_SIZE 10
#define FPS 60

#define VERT_PIN A0
#define HORZ_PIN A1
#define SEL_PIN  2

struct Point2d
{
    Point2d()
        : x(0)
        , y(0)
    {}

    Point2d(float anX, float anY)
        : x(anX)
        , y(anY)
    {
    }

    void print()
    {
        Serial.print("{ x: ");
        Serial.print(x);
        Serial.print(", y: ");
        Serial.print(y);
        Serial.println(" }");
    }

    float x;
    float y;
};

struct Point3d
{
    Point3d()
        : x(0)
        , y(0)
        , z(0)
    {}

    Point3d(const Point3d &aPoint)
        : x(aPoint.x)
        , y(aPoint.y)
        , z(aPoint.z)
    {}

    Point3d(float anX, float anY, float aZ)
        : x(anX)
        , y(anY)
        , z(aZ)
    {
    }

    void print()
    {
        Serial.print("{ x: ");
        Serial.print(x);
        Serial.print(", y: ");
        Serial.print(y);
        Serial.print(", z: ");
        Serial.print(z);
        Serial.println(" }");
    }

    float x;
    float y;
    float z;
};


#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)
Adafruit_SSD1306 display(128, 64, &Wire, OLED_RESET); //Declaration for the size,setting,etc. of the OLED

void point(const Point2d &aPoint)
{
    display.fillRect(aPoint.x - POINT_SIZE / 2, aPoint.y - POINT_SIZE / 2, 10, 10, SSD1306_WHITE);
}

void line(const Point2d &aP1, const Point2d &aP2)
{
    display.drawLine(aP1.x, aP1.y, aP2.x, aP2.y, SSD1306_WHITE);
}

Point2d screen(const Point2d &aPoint)
{
    Point2d result;

    result.x = (aPoint.x + 1) / 2 * SCREEN_WIDTH;
    result.y = (1 - (aPoint.y + 1) / 2) * SCREEN_HEIGHT;

    // result.print();

    return result;
}

Point2d project_3d_to_2d(const Point3d &aPoint)
{
    Point2d result;

    result.x = aPoint.x / aPoint.z;
    result.y = aPoint.y / aPoint.z;

    return result;
}

Point3d vertixes[] = 
    {
        {-0.4,  0.4,  0.4}, // 0
        { 0.4,  0.4,  0.4}, // 1
        { 0.4, -0.4,  0.4}, // 2
        {-0.4, -0.4,  0.4},

        {-0.4,  0.4, -0.4},
        { 0.4,  0.4, -0.4},
        { 0.4, -0.4, -0.4},
        {-0.4, -0.4, -0.4}
    };

int fs[] = 
    {
        0, 1, 2, 3,
        0, 4, 5, 6, 7,
        4, 0, 
        1, 5,
        6, 2, 
        3, 7, 4
    };

int vSize = 0;
int fsSize = 0;

float dt = 0;
float dz = 1;
float angleXZ = 0;
float angleYZ = 0;

void setup()
{
    Serial.begin(9600); // Start serial communication at 9600 baud
    Serial.println("Debugging started...");

    pinMode(VERT_PIN, INPUT);
    pinMode(HORZ_PIN, INPUT);
    pinMode(SEL_PIN, INPUT_PULLUP);

    if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) 
    {
        Serial.println(F("SSD1306 allocation failed"));
        for(;;);
    }
    dt = 1.0 / FPS;
    vSize = (sizeof(vertixes) / sizeof(vertixes[0]));
    fsSize = (sizeof(fs) / sizeof(fs[0]));
}

Point3d translate_z(const Point3d &aPoint, float dz)
{
    Point3d result = aPoint;
    result.z = result.z + dz;
    
    return result;
}

Point3d rotate_xz(const Point3d &aPoint, float anAngle)
{
    Point3d result = aPoint;

    float c = cos(anAngle);
    float s = sin(anAngle);

    result.x = (result.x * c) - (result.z * s);
    result.z = (result.x * s) - (result.z * c);
    
    return result;
}

Point3d rotate_yz(const Point3d &aPoint, float anAngle)
{
    Point3d result = aPoint;

    float c = cos(anAngle);
    float s = sin(anAngle);

    result.y = (result.y * c) - (result.z * s);
    result.z = (result.y * s) - (result.z * c);
    
    return result;
}

void loop() 
{
    display.clearDisplay();

    int vert = analogRead(VERT_PIN);
    int horz = analogRead(HORZ_PIN);
    bool selPressed = digitalRead(SEL_PIN) == LOW;

    if (selPressed)
    {
        angleXZ = 0;
        angleYZ = 0;
    }

    float YZmultiplier = ((float)vert - 512) / 512;
    float XZmultiplier = ((float)horz - 512) / 512;

    angleXZ = angleXZ + (XZmultiplier * PI * dt);
    angleYZ = angleYZ + ((0 - YZmultiplier) * PI * dt);

    for (int iEdge = 0; iEdge < fsSize; iEdge++)
    {
        int index_p1 = fs[iEdge];
        int index_p2 = fs[(iEdge+1) % fsSize];
        Point3d p1 = vertixes[index_p1];
        Point3d p2 = vertixes[index_p2];

        p1 = rotate_xz(p1, angleXZ);
        p1 = rotate_yz(p1, angleYZ);

        p2 = rotate_xz(p2, angleXZ);
        p2 = rotate_yz(p2, angleYZ);

        line(
          screen(project_3d_to_2d(translate_z(p1, 1) )),
          screen(project_3d_to_2d(translate_z(p2, 1) ))
        );
    }

    display.display();
}

          // screen(project_3d_to_2d(translate_z( rotate_yz(p1, angle), dz))),
          // screen(project_3d_to_2d(translate_z( rotate_yz(p2, angle), dz)))
