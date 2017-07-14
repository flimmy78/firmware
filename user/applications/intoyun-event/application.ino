/*
 * InotYun 测试程序
 */

//SerialUSBDebugOutput debugOutput(115200, ALL_LEVEL);
SerialDebugOutput debugOutput(115200, ALL_LEVEL);

PRODUCT_ID(y4NFFyDE9uq6H202)     //产品ID
PRODUCT_SECRET(ab697b0dc1716d24cfc49b071668e766) //产品密钥
PRODUCT_VERSION(2)     //产品版本号

#define DPID_ENUM_LIGHT_MODE             1        //颜色模式
#define DPID_NUMBER_TEMPERATURE          2        //温度
#define DPID_BOOL_SWITCH                 3        //灯泡开关
#define DPID_BOOL_LIGHT_STATUS           4        //灯泡亮灭状态
#define DPID_NUMBER_RHEOSTAT             5        //速度
#define DPID_STRING_LCD_DISPLAY          6        //字符显示
#define DPID_BINARY_DATA                 7        //透传数据

#define LEDPIN    LED_USER    //例子灯

int LightMode;
double Temperature;
bool LightSwitch;
bool LightStatus;
int Rheostat_tmp;
int Rheostat;
String LcdDisplay;

void system_event_callback(system_event_t event, int param, uint8_t *data, uint16_t datalen)
{
    Serial.printf("system_event_callback");
    if((event == event_cloud_data) && (param == ep_cloud_data_datapoint)) {
        // 颜色模式
        if (RESULT_DATAPOINT_NEW == IntoRobot.readDatapoint(DPID_ENUM_LIGHT_MODE, LightMode))
        {
            Serial.printf("Read Light Mode: ");
            switch(LightMode)
            {
                case 0:
                    Serial.printf("GREEN");
                    break;
                case 1:
                    Serial.printf("RED");
                    break;
                case 2:
                    Serial.printf("BLUE");
                    break;
                default:
                    Serial.printf("Error");
                    break;
            }
            Serial.printf("\r\n");
        }
        //灯泡控制
        if (RESULT_DATAPOINT_NEW == IntoRobot.readDatapoint(DPID_BOOL_SWITCH, LightSwitch))
        {
            Serial.printf("switch: %d\r\n", LightSwitch);
            if(true == LightSwitch)
            {
                digitalWrite(LEDPIN, LOW);     // 打开灯泡
                IntoRobot.writeDatapoint(DPID_BOOL_LIGHT_STATUS, true);
            }
            else
            {
                digitalWrite(LEDPIN, HIGH);    // 关闭灯泡
                IntoRobot.writeDatapoint(DPID_BOOL_LIGHT_STATUS, false);
            }
        }

        //速度控制
        if (RESULT_DATAPOINT_NEW == IntoRobot.readDatapoint(DPID_NUMBER_RHEOSTAT, Rheostat_tmp))
        {
            Rheostat = Rheostat_tmp;
            Serial.printf("Speed: %d\r\n", Rheostat);
        }

        //字符串显示
        if (RESULT_DATAPOINT_NEW == IntoRobot.readDatapoint(DPID_STRING_LCD_DISPLAY, LcdDisplay))
        {
            Serial.printf("Lcd Display: %s\r\n", LcdDisplay.c_str());
        }
    }
}

void setup()
{
    Serial.begin(115200);
    pinMode(LEDPIN, OUTPUT);
    IntoRobot.defineDatapointEnum(DPID_ENUM_LIGHT_MODE, DP_PERMISSION_UP_DOWN, 0);                          //颜色模式
    IntoRobot.defineDatapointNumber(DPID_NUMBER_TEMPERATURE, DP_PERMISSION_UP_ONLY, -100, 100, 2, 0);       //温度
    IntoRobot.defineDatapointBool(DPID_BOOL_SWITCH, DP_PERMISSION_UP_DOWN, false);                          //灯泡开关
    IntoRobot.defineDatapointBool(DPID_BOOL_LIGHT_STATUS, DP_PERMISSION_UP_ONLY, false);                    //灯泡亮灭状态
    IntoRobot.defineDatapointNumber(DPID_NUMBER_RHEOSTAT, DP_PERMISSION_UP_DOWN, 0, 1000, 0, 0);            //速度
    IntoRobot.defineDatapointString(DPID_STRING_LCD_DISPLAY, DP_PERMISSION_UP_DOWN, 255, "oh yeah!");       //字符显示
    IntoRobot.defineDatapointBinary(DPID_BINARY_DATA, DP_PERMISSION_UP_DOWN, 255, "\x23\x32\x32\x43", 4);   //字符显示
    System.on(event_cloud_data, &system_event_callback);
}

void loop()
{

    //温度上送
    if(Temperature > 100)
    {Temperature = 0;}
    else
    {Temperature += 0.1;}
    IntoRobot.writeDatapoint(DPID_NUMBER_TEMPERATURE, Temperature);

    //速度上送
    if(Rheostat > 1000)
    {Rheostat = 0;}
    else
    {Rheostat += 5;}
    IntoRobot.writeDatapoint(DPID_NUMBER_RHEOSTAT, Rheostat);

    IntoRobot.sendDatapointAll();
    delay(10000);
}

