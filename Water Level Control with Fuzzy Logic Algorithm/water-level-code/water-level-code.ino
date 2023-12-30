#include<math.h>
#include<stdio.h>
#include<ThingerESP32.h>
#define USERNAME "Naufal312"
#define DEVICE_ID "ESP32_SoilSensor"
#define DEVICE_CREDENTIAL "oumiM%#hQlR#$y&x"

#define SSID "Naufal"
#define SSID_PASSWORD "ichbinexschuler"

ThingerESP32 thing(USERNAME, DEVICE_ID, DEVICE_CREDENTIAL);

//Deklarasi pin Sensor
float durasi, jarak, ketinggian, moistRead;
#define trig 26
#define echo 27
#define soilSensor  35
#define relay1 19
#define relay2 18

//Deklarasi variable untuk program fuzzy
float fuInput, fuOutput;
float a1, b1, b2, c1;
float A;
float B;
int sel_;
float C1, C2, C3, C4, C5, C6, C7;
float M1, M2, M3, M4, M5, M6, M7;

//Himpunan Fuzzy Input
float kering[4]={0, 0, 5, 30};
float lembab[3]={15, 50, 85};
float basah[4]={70, 95, 100, 100};

//Himpunan Fuzzy Output
float rendah[3]={0, 0, 1.5};
float medium[3]={1, 3, 5};
float tinggi[3]={4.5, 6, 6};

void setup() {
pinMode(relay1,OUTPUT);
pinMode(relay2,OUTPUT);
pinMode(trig,OUTPUT);
pinMode(echo,INPUT);
pinMode(soilSensor, INPUT);
  
thing.add_wifi(SSID, SSID_PASSWORD);
thing["Soil Sensor"] >> [](pson& out){
  out["Kelembaban"] = fuInput;
  out["Ketinggian air"] = ketinggian;
};
Serial.begin(115200);
}

//Rumus Membership Function
float FuIn1(){
  if (fuInput<kering[2]){
    return 1;
  }
  else if (kering[2]<=fuInput && fuInput<=kering[3]){
    return ((fuInput-kering[2])/(kering[3]-kering[2]));
  }
  else if (fuInput>kering[3]){
    return 0;
  }
}
  
float FuIn2(){
  if (fuInput<lembab[0]){
    return 0;
  }
  else if (lembab[0]<=fuInput && fuInput <=lembab[1]){
    return ((fuInput-lembab[0])/(lembab[1]-lembab[0]));
  }
  else if (lembab[1]<=fuInput && fuInput<=lembab[2]){
    return ((lembab[2]-fuInput)/(lembab[2]-lembab[1]));
  }
  else if (fuInput>lembab[2]){
    return 0;
  }
}

float FuIn3(){
  if (fuInput<basah[0]){
    return 0;
  }
  else if (basah[0]<=fuInput && fuInput<=basah[1]){
    return ((fuInput-basah[0])/(basah[1]-basah[0]));
  }
  else if (fuInput>basah[1]){
    return 1;
  }
}

//Output
float FuOut1(){
 if  (fuOutput<rendah[1]){
  return 1;
 }
 else if (rendah[1]<=fuOutput && fuOutput<=rendah[2]){
  return ((fuOutput-rendah[1])/(rendah[2]-rendah[1]));
 }
 else if (fuOutput>rendah[2]){
  return 0;
 }
}

float fuOut2(){
 if (fuOutput<medium[0]){
  return 0; 
 }
 else if (medium[0]<=fuOutput && fuOutput<=medium[1]){
  return ((fuOutput-medium[0])/(medium[1]-medium[0]));
 }
 else if (medium[1]<=fuOutput && fuOutput<=medium[2]){
  return ((medium[2]-fuOutput)/(medium[2]-medium[1]));
 }
 else if (fuOutput>medium[2]){
  return 0;
 }
}

float fuOut3(){
  if (fuOutput<tinggi[0]){
    return 0;
  }
  else if (tinggi[0]<=fuOutput && fuOutput<=tinggi[1]){
    return ((fuOutput-tinggi[0])/(tinggi[1]-tinggi[0]));
  }
  else if (fuOutput>tinggi[1]){
    return 1;
  }
}

void implikasi(){
  c1 = 1.5 - (FuIn3()*(rendah[2]-rendah[1]));
  b2 = 1 + (FuIn2()*(medium[1]-medium[0]));
  b1 = 5 - (FuIn2()*(medium[2]-medium[1]));
  a1 = 4.5 + (FuIn1()*(tinggi[1]-tinggi[0]));
}

float f (float x){
  if (B>0 && sel_==0){
    return ((x-A)/B)*x;
  }
  else if (B>0 && sel_==1){
    return ((A-x)/B)*x;
  }
  else {
    return A*x;
  }
}

float simpsons (float f(float x), float a, float b, float n){
  float h, integral, x, sum=0;
  int i;
  h=fabs(b-a)/n;
  for (i=1; i<n; i++){
    x=a+i*h;
    if (i%2==0){
      sum=sum+2*f(x);
    }
    else {
      sum=sum+4*f(x);
    }
  }
  integral=(h/3)*(f(a)+f(b)+sum);
  return integral;
}

float fx (float limd, float limu, float a, float b, int sel){
  int n,i=2;
  float h, x, integral, eps=0.1, integral_new;
  A = a;
  B = b;
  sel_ = sel;

  integral_new=simpsons(f,limd,limu,i);
  do{
    integral=integral_new;
    i=i+2;
    integral_new=simpsons(f,limd,limu,i);
  }
  while (fabs(integral_new-integral)>=eps);
  return integral_new;
}

void luas_deFuzzify(){
  implikasi();
  C1 = ((rendah[2]-c1)*FuIn3())/2;
  C2 = (c1 - rendah[0])*FuIn3();
  C3 = ((b2 - medium[0])*FuIn2())/2;
  C4 = ((medium[2] - b1)*FuIn2())/2;
  C5 = (b1 - b2)*FuIn2();
  C6 = ((a1 - tinggi[0])*FuIn1())/2;
  C7 = (tinggi[2] - a1)*FuIn1();
}

void moment(){
  luas_deFuzzify();
  M1 = fx(c1, rendah[2], rendah[2], (rendah[2]-rendah[0]),1);
  M2 = fx(rendah[0], c1, FuIn3(), 0, 0);
  M3 = fx(medium[0], b2, medium[0], (medium[1]-medium[0]), 0);
  M4 = fx(b1, medium[2], medium[2], (medium[2]-medium[1]), 1);
  M5 = fx(b2, b1, FuIn2(), 0, 0);
  M6 = fx(tinggi[0], a1, tinggi[0], (tinggi[2]-tinggi[0]), 0);
  M7 = fx(a1, tinggi[2], FuIn1(), 0, 0);
}

float deFuzzify(){
  moment();
  return ((M1+M2+M3+M4+M5+M6+M7)/(C1+C2+C3+C4+C5+C6+C7));
}


void loop() {
  moistRead = analogRead(soilSensor);
 
  fuInput = (100-((moistRead/4095.00)*100));

  digitalWrite(trig,LOW);
  delayMicroseconds(8);
  digitalWrite(trig,HIGH);
  delayMicroseconds(8);
  digitalWrite(trig,LOW);
  delayMicroseconds(8);
  durasi = pulseIn(echo,HIGH);
  jarak = (0.034*(durasi/2));
  ketinggian = (16.5-jarak);
  

  if (ketinggian<deFuzzify()){
    digitalWrite(relay1,HIGH);
    digitalWrite(relay2,LOW);
  }
  else if (ketinggian>deFuzzify()){
    digitalWrite(relay1,LOW);
    digitalWrite(relay2,HIGH);
  }
  else {
    digitalWrite(relay1,LOW);
    digitalWrite(relay2,LOW);
  }
  Serial.print("Kelembaban=");
  Serial.println(fuInput);
  Serial.print("Ketinggian =");
  Serial.println(ketinggian);
  Serial.print("Jarak =");
  Serial.println(jarak);
  Serial.print("Output Fuzzy =");
  Serial.println(deFuzzify());
  delay(500);
  thing.handle();
}
