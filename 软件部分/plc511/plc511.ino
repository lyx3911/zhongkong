#define trig1 22//上面的超声
#define echo1 23
#define trig2 36//下面的超声
#define echo2 37
#define DISTANCETHRE 15

const int M1 = 4,M2 = 5,M3 = 3,M4 = 2;//左边1和2，右边3和4；
double UM1=131,UM2=132;
const int infrared[5]={A4,A3,A2,A1,A0};
const int infraredL[5] = {A10,A11,A12,A13,A14};
const int infraredR[5] = {A5,A6,A7,A8,A9};
 
int infrared_val[5];
double line_mid,line_mid_L,line_mid_R;
int in_cross = 0;//0表示不在交叉处，1表示在
int crood_x = 0,crood_y = 0;//出发方向左侧为x轴正方向，前方为y轴正方向
int direction=1;//direction%4==0为x轴正方向，1为y轴正方向，2为x轴负方向，3为y轴负方向，arduino用%会有负数结果
int turning_mode=0;

char serial_line[100] ="";
int serial_line_length=0;

const int servo[5] = {13,12,11,10,9};//舵机编号
int angle[5] = {0,0,20,180,65};//舵机角度p

bool lattice[4][12]={0};//记录格子是否有物品，1为有，0为无；0-5下层，6-11为上层

bool isMaterial(double distance){
  return distance>DISTANCETHRE;
}

double DistanceMeasure1()
{
  int returntime=0,distance=0;
  digitalWrite(trig1,HIGH);
  delayMicroseconds(10);
  digitalWrite(trig1,LOW);
  returntime=pulseIn(echo1,HIGH);
  distance=returntime*0.034/2;
  return distance;
}

double DistanceMeasure2()
{
  int returntime=0,distance=0;
  digitalWrite(trig2,HIGH);
  delayMicroseconds(10);
  digitalWrite(trig2,LOW);
  returntime=pulseIn(echo2,HIGH);
  distance=returntime*0.034/2;
  return distance;
}

void move_to(int fx,int fy)
{
  int dx=fx-crood_x;
  int dy=fy-crood_y;
  d_move_to(dx,dy);
}

void d_move_to(int dx,int dy)
{
  int d_direction=0;
  int final_x=crood_x+dx;
  int final_y=crood_y+dy;
  int tem;
  if(dx==0&&dy==0) return;
  if(dx >0&&dy==0) d_direction=0;
  if(dx >0&&dy >0) d_direction=1;
  if(dx==0&&dy >0) d_direction=2;
  if(dx <0&&dy >0) d_direction=3;
  if(dx <0&&dy==0) d_direction=4;
  if(dx <0&&dy <0) d_direction=5;
  if(dx==0&&dy <0) d_direction=6;
  if(dx >0&&dy <0) d_direction=7;

  switch(direction)
  {
    case 1:
      tem=dx;
      dx=dy;
      dy=-tem;
      break;
    case 2:
      dx=-dx;
      dy=-dy;
      break;
    case 3:
      tem=dx;
      dx=-dy;
      dy=tem;
      break;
  }

  switch((d_direction-direction*2+8)%8)
  {
    case 0:
      move_to_cross(dx);
      break;
    case 1:
      move_to_cross(dx);
      turn_left();
      move_to_cross(dy);
      break;
    case 2:
      turn_left();
      move_to_cross(dy);
      break;
    case 3:
      turn_left();
      move_to_cross(dy);
      turn_left();
      move_to_cross(dx);
      break;
    case 4:
      turn_back();
      move_to_cross(dx);
      break;
    case 5:
      turn_right();
      move_to_cross(dy);
      turn_right();
      move_to_cross(dx);
      break;
    case 6:
      turn_right();
      move_to_cross(dy);
      break;
    case 7:
      move_to_cross(dx);
      turn_right();
      move_to_cross(dy);
      break;
  }
}

void move_to_cross(int cross_num)
{
  int sumxy=crood_x+crood_y;
  while(abs(crood_x+crood_y-sumxy)<abs(cross_num))
    {
       go_forward();
    }
   
    rapid_stop();
    delay(1000);
}
void moveback_to_cross(int cross_num)
{
  int sumxy=crood_x+crood_y;
  while(abs(crood_x+crood_y-sumxy)<abs(cross_num))
    {
       go_backward();
    }
   
    stop();
    delay(2000);
}
void count_cross()
{
  //扫到白线
    if(fabs(line_mid_L)>100&&fabs(line_mid_R)>100&&in_cross==0)
   {
    in_cross=1;

    if(turning_mode)
      turning_mode=0;
    else if((10000+direction)%4==0)
      crood_x++;
    else if((10000+direction)%4==1)
      crood_y++;
    else if((10000+direction)%4==2)
      crood_x--;
    else if((10000+direction)%4==3)
      crood_y--;
//    Serial.println("\n crood_x: ");
//    Serial.println(crood_x);
//    Serial.println("\n crood_y: ");
//    Serial.println(crood_y);
//    Serial.println("\n turning_mode: ");
//    Serial.println(turning_mode);
//    Serial.println("\n direction%4 ");
//    Serial.println((10000+direction)%4);
   }
   
//   Serial.println(infrared_val[2]);
   //离开白线
    if(fabs(line_mid_L)<10&&fabs(line_mid_R)<10&&in_cross==1&&infrared_val[2]>500)
   {
    in_cross = 0;
//    Serial.println(" crood_x: ");
//    Serial.println(crood_x);
   }
}

void turn_back()
{
  turn_right();
  in_cross = 0; 
  int flag =0;
  while(1)
  {    
    get_line_mid();
    if(flag==0 && fabs(line_mid)>1000)
    flag=1;
    
    analogWrite(M1,0);
    analogWrite(M2,UM1+10);
    analogWrite(M3,0);
    analogWrite(M4,UM2+10);

    
    if(flag==1 && fabs(line_mid)<100)
    flag=2;
    if(flag==2 && fabs(line_mid)>300)
    break;
  } 
  
    double time0=millis();
  while(millis()-time0<200)
  {
    analogWrite(M1,0);
    analogWrite(M2,UM1+10);
    analogWrite(M3,0);
    analogWrite(M4,UM2+10);

  }
  turning_mode=1;
  direction--;
 
}

void turn_right()
{
  while(1)
  {
    
    if(fabs(line_mid_L)>100||fabs(line_mid_R)>100||fabs(infrared_val[2])<500)
    {
      analogWrite(M2,0);
      analogWrite(M1,UM1-5);
      analogWrite(M4,UM2-5);
      analogWrite(M3,0);
      get_line_mid();
    }
    else
    {
      delay(5);
      get_line_mid();
      if(!(fabs(line_mid_L)>100||fabs(line_mid_R)>100)) break;
    }
  }
  stop();
  in_cross = 0; 
  int flag =0;
  while(1)
  {    
    get_line_mid();
    if(flag==0 && fabs(line_mid)>1000)
    flag=1;
    
    analogWrite(M1,0);
    analogWrite(M2,UM1+10);
    analogWrite(M3,0);
    analogWrite(M4,UM2+10);

    
    if(flag==1 && fabs(line_mid)<300)
    flag=2;
    if(flag==2 && fabs(line_mid)>500)
    break;
  } 
  
    double time0=millis();
  while(millis()-time0<200)
  {
    analogWrite(M1,0);
    analogWrite(M2,UM1+10);
    analogWrite(M3,0);
    analogWrite(M4,UM2+10);

  }
  turning_mode=1;
}

void turn_left()
{
  while(1)
  {
    if(fabs(line_mid_L)>100||fabs(line_mid_R)>100||fabs(infrared_val[2])<500)
    {
      analogWrite(M2,0);
      analogWrite(M1,UM1-5);
      analogWrite(M4,UM2-5);
      analogWrite(M3,0);
      get_line_mid();
    }
    else
    {
      delay(5);
      get_line_mid();
      if(!(fabs(line_mid_L)>100||fabs(line_mid_R)>100)) break;
    }
  }
  stop();
  in_cross = 0; 
  int flag =0;
  while(1)
  {    
    get_line_mid();
    if(flag==0&&fabs(line_mid)>1000)
    {
//      Serial.println(flag);
//      Serial.println(line_mid);
      flag=1;
//      stop();
//      delay(2000);
    }
    
    
    analogWrite(M1,UM1);
    analogWrite(M2,0);
    analogWrite(M3,UM2);
    analogWrite(M4,0);

//    Serial.println(flag);
//    Serial.println(line_mid);
    if(flag==1&&fabs(line_mid)<200)
    {
      flag=2;
//      Serial.println(flag);
//      Serial.println(line_mid);
//      stop();
//      delay(2000);    
//      analogWrite(M1,UM1+30);
//      analogWrite(M2,0);
//      analogWrite(M3,UM2+30);
//      analogWrite(M4,0);  
    }
    
    if(flag==2&&fabs(line_mid)>300)
    {
      stop();
//      Serial.println(flag);
//      Serial.println(line_mid);
//      delay(2000);
      break;
    }
    double time0=millis();
  while(millis()-time0<200)
  {
    analogWrite(M1,UM1);
    analogWrite(M2,0);
    analogWrite(M3,UM2);
    analogWrite(M4,0);

  }
  } 
  turning_mode=1;
}

void get_line_mid()
{
  line_mid = 0;
  for(int i=0;i<5;i++)
  {
    //黑色1000；白色0；
    infrared_val[i] = 1000-analogRead(infrared[i]);
//   Serial.println(infrared_val[i]);
//   Serial.println(" ");
   line_mid += infrared_val[i]*(i-2);
  }
  line_mid_L = 0;
  for(int i =0 ;i<5;i++)
  {
    infrared_val[i] = 1000-analogRead(infraredL[i]);
//   Serial.println(infrared_val[i]);
//   Serial.println(" ");
   line_mid_L += infrared_val[i]*(i-2);
  }
  line_mid_R = 0;
  for(int i =0 ;i<5;i++)
  {
    infrared_val[i] = 1000-analogRead(infraredR[i]);
//   Serial.println(infrared_val[i]);
//   Serial.println(" ");
   line_mid_R += infrared_val[i]*(i-2);
  }
//    Serial.println(line_mid);
//    Serial.println(" ");
//    Serial.println(line_mid_L);
//    Serial.println(" ");
//    Serial.println(line_mid_R);
//    Serial.println("\n");
}

void stop ()
{
  digitalWrite(M1,1);
  digitalWrite(M2,1);
  digitalWrite(M3,1);
  digitalWrite(M4,1);
}
void rapid_stop()
{
  analogWrite(M2,0);
  analogWrite(M1,UM1);
  analogWrite(M4,UM2);
  analogWrite(M3,0);
  delay(15);
  stop();
}
void go_forward()
{
  get_line_mid();
  analogWrite(M1,0);
  analogWrite(M2,UM1+line_mid/50);
  analogWrite(M3,UM2-line_mid/50);
  analogWrite(M4,0);

  count_cross();
}
void go_backward()
{
  get_line_mid();
  analogWrite(M2,0);
  analogWrite(M1,UM1);
  analogWrite(M4,UM2);
  analogWrite(M3,0);

  count_cross();
}
void fuck_wall() {
  get_line_mid();
  analogWrite(M1,0);
  analogWrite(M2,UM1-line_mid/10);
  analogWrite(M3,UM2+line_mid/50);
  analogWrite(M4,0);
  delay(1200);
  stop();
}

int CharToInt(char c)
{
  return c-'0';
}

void PC_to_Arduino()
{
  int goto_x=0,goto_y=0;
  if (Serial.available() > 0)
  {
    // 读取，读到\n或100字符或超时
    serial_line_length = Serial.readBytesUntil('\n', serial_line, 100);
    serial_line[serial_line_length ]='\0';    // 截断字符串
    String input = serial_line;
    char* input_sign_c=new char[100];
    strcpy(input_sign_c,serial_line);
    input_sign_c[4]='\0';

    switch (input_sign_c[0]) {
      case 'a':
        switch (input_sign_c[1]) {
          case '0':
            predict_left();
            break;
          case '1':
            predict_mid();
            break;
          case '2':
            predict_right();
            break;
        }
        Serial.println("finished");
        break;

      case 'b':
        switch (input_sign_c[1]) {
          case '0':
            take_left();
            break;
          case '1':
            take_mid();
            break;
          case '2':
            take_right();
            break;
        }
        Serial.println("finished");
        break;

      case 'c':
        switch (input_sign_c[1]) {
          case '0':
            put_lower();
            break;
          case '1':
            put_upper();
            break;
        }
        Serial.println("finished");
        break;

      case 'm':
        move_to_cross(input_sign_c[1] - '0');
        Serial.println("finished");
        break;


      default:
      if(input_sign_c[0]=='e'){
          Serial.println(check_windows());
        }
      if(input_sign_c[0]=='t'){
          int tgtDir = input_sign_c[1] - '0';
          if ((tgtDir + 1) % 4 == direction) {
            turn_left();
          } else if ((direction + 1) % 4 == tgtDir) {
            turn_right();
          }
          direction = tgtDir;
          stop();
          Serial.println("finished");
      }
  
      if(input_sign_c[0]=='d'){
        if (DistanceMeasure1() < 80) {
          Serial.println(1);
        } else {
          Serial.println(0);
        }
      }
      
      
    }
  }
}

//舵机部分
void servopulse(int serv, int myangle)/*定义一个脉冲函数，用来模拟方式产生PWM值*/
{
  int pulsewidth=(myangle*11)+500;//将角度转化为500-2480 的脉宽值
  digitalWrite(serv,HIGH);//将舵机接口电平置高
  delayMicroseconds(pulsewidth);//延时脉宽值的微秒数
  digitalWrite(serv,LOW);//将舵机接口电平置低
  delay(20-pulsewidth/1000);//延时周期内剩余时间
}



//平滑改变舵机角度
void soft_change_angle(int x, int new_angle)//需要改变的舵机编号,改变后的角度
{
  //其他舵机保持角度不变
  for(int i = 0;i<4;i++)
  {
    if(x!=i)
    servopulse(servo[i], angle[i]);
  }
  
    if (angle[x] < new_angle)
    {
      for(int i = angle[x];i <= new_angle;i++)
      {
        servopulse(servo[x], i);
        delay((i-angle[x])/5);
      }
    }
    else if(angle[x]>new_angle)
    {
      for(int i = angle[x];i>=new_angle;i--)
      {
        servopulse(servo[x], i);
        delay((angle[x]-i)/5);
      }
    }
    angle[x] = new_angle;
}

void take_mid()
{
  soft_change_angle(0,0);
  soft_change_angle(1,5);
  soft_change_angle(2,50);
  soft_change_angle(3,10);
  double time0=millis();
  while(millis()-time0<600)
  {
    go_forward();
  }
  soft_change_angle(0,70);
  soft_change_angle(3,90);
  
  soft_change_angle(1,0);
  soft_change_angle(2,20);
  soft_change_angle(3,180);
  moveback_to_cross(1);
}

void take_left()
{
  soft_change_angle(0,0);
  soft_change_angle(4,100);
  soft_change_angle(1,20);
  soft_change_angle(2,70);
  soft_change_angle(3,10);
  delay(300);
  double time0=millis();
  while(millis()-time0<1500)
  {
    go_forward();
  }
//  soft_change_angle(3,10);
  soft_change_angle(0,70);
  soft_change_angle(3,90);
  soft_change_angle(4,65);
  soft_change_angle(1,0);
  soft_change_angle(2,20);
  soft_change_angle(3,180);
  moveback_to_cross(1);
  stop();
}

void take_right()
{
  soft_change_angle(0,0);
  soft_change_angle(4,30);
  soft_change_angle(1,20);
  soft_change_angle(2,70);
  soft_change_angle(3,10);
  delay(300);
  double time0=millis();
  while(millis()-time0<1500)
  {
    go_forward();
  }
  soft_change_angle(0,70);
  soft_change_angle(3,90);
  soft_change_angle(4,65);
  soft_change_angle(1,0);
  soft_change_angle(2,20);
  soft_change_angle(3,180);
  moveback_to_cross(1);
  stop();
}

void predict_mid()
{
  soft_change_angle(0,0);
  soft_change_angle(4,65);
  soft_change_angle(1,90);
  soft_change_angle(2,20);
  soft_change_angle(3,90);
  delay(300);
}

void predict_left()
{
  soft_change_angle(0,0);
  soft_change_angle(4,85); 
  soft_change_angle(1,15);
  soft_change_angle(2,65);
  soft_change_angle(3,0);
  delay(300);
}

void predict_right()
{
  soft_change_angle(0,0);
  soft_change_angle(4,45);
  soft_change_angle(1,15);
  soft_change_angle(2,65);
  soft_change_angle(3,0);
  delay(300);
}
void hold()
{
  servopulse(servo[0],angle[0]);
  servopulse(servo[1],angle[1]);
  servopulse(servo[2],angle[2]);
  servopulse(servo[3],angle[3]);
  servopulse(servo[4],angle[4]);
}
void put_upper()
{
  soft_change_angle(0,70);
  soft_change_angle(3,140);
  soft_change_angle(2,85);
  soft_change_angle(1,125);
  soft_change_angle(3,100);
  soft_change_angle(0,0);
  moveback_to_cross(1);
  stop();
  soft_change_angle(1,0);
  soft_change_angle(2,20);
  soft_change_angle(3,180);
  moveback_to_cross(1);
}
void put_lower()
{
  soft_change_angle(0,70);
  moveback_to_cross(1);
  soft_change_angle(2,60);
  soft_change_angle(3,10);
  soft_change_angle(1,0);
  double time0=millis();
  while(millis()-time0<1800)
  {
    go_forward();
  }
  stop();
  soft_change_angle(2,45);
  soft_change_angle(0,0);
  moveback_to_cross(1);
  stop();
  soft_change_angle(1,0);
  soft_change_angle(2,20);
  soft_change_angle(3,180);
}
int check_windows()
{
  /*
   如果上面空放上面，下面空放下面，优先上面，都不空执行move_back_to_cross
   */
//  move_to_cross(1);
  soft_change_angle(0,70);
  double time0=millis();
  while(millis()-time0<1800)
  {
    go_forward();
  }
  stop();
  soft_change_angle(1,5);
  soft_change_angle(2,50);
  soft_change_angle(3,100);
  soft_change_angle(4,65);
  delay(2000);
  int res = 0;
  if (DistanceMeasure1 < DISTANCETHRE) {
    res += 1;
  }
  if (DistanceMeasure2 < DISTANCETHRE) {
    res += 2;
  }
  return res;
  /*
  Serial.println(DistanceMeasure1());
  Serial.println(DistanceMeasure2());
  */
}
void setup() 
{
  // put your setup code here, to run once:
  pinMode(M1,OUTPUT);
  pinMode(M2,OUTPUT);
  pinMode(M3,OUTPUT);
  pinMode(M4,OUTPUT);
  Serial.begin(9600);

  pinMode(servo[0], OUTPUT);
  pinMode(servo[1], OUTPUT);
  pinMode(servo[2], OUTPUT);
  pinMode(servo[3], OUTPUT);
  pinMode(servo[4],OUTPUT);
  //delay(2000);
  for(int i = 0;i<5;i++)
  {
    pinMode(infrared[i],INPUT);
    pinMode(infraredL[i],INPUT);
    pinMode(infraredR[i],INPUT);
  }

  pinMode(trig1,OUTPUT);
  pinMode(echo1,INPUT);
  pinMode(trig2,OUTPUT);
  pinMode(echo2,INPUT);
  
  servopulse(servo[0],angle[0]);
  servopulse(servo[1],angle[1]);
  servopulse(servo[2],angle[2]);
  servopulse(servo[3],angle[3]);
  servopulse(servo[4],angle[4]);
  delay(2000);
 
  Serial.setTimeout(1000); //串口超时 1000 毫秒 
}


void loop() {
  //check_windows();
  //delay(10000);
  PC_to_Arduino();
  /*
  move_to_cross(4);
  turn_right();
  move_to_cross(1);
  turn_left();
  move_to_cross(1);
  turn_left();
  move_to_cross(1);
  stop();
  delay(1000);
  predict_right();
  stop();
//  soft_change_angle(3, 45);
//  double time0=millis();
//  
//  stop();
//  delay(2000);
  take_right();
  moveback_to_cross(1);
  turn_right();
  move_to_cross(1);
  turn_right();
  move_to_cross(1);
  stop();
  delay(2000);
  check_windows();
  put_lower();
  stop();
  delay(10000);
  
//  soft_change_angle(1,5);
//  soft_change_angle(2,50);
////  soft_change_angle(0,60);
//  soft_change_angle(3,100);
//  soft_change_angle(4,65);
//  double time0=millis();
//  while(millis()-time0<900)
//  {
//    go_forward();
//  }
//  stop();
//  delay(2000);
//  
//  Serial.println(DistanceMeasure1());
//  Serial.println(DistanceMeasure2());
//  soft_change_angle(3,140);
//  soft_change_angle(2,85);
//  soft_change_angle(1,125);
//  soft_change_angle(3,100);
//  soft_change_angle(0,0);
//  moveback_to_cross(1);
//  stop();
//  delay(10000);
  /*
  in_cross = 1;
  moveback_to_cross(1);
  turn_left();
  move_to_cross(1);
  turn_right();
  move_to_cross(1);
  double time0=millis();
  while(millis()-time0<900)
  {
    go_forward();
  }
  stop();
  */
  
}
