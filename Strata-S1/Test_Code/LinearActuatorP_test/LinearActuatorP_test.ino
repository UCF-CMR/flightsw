// This is test code for operating the P-Type Actuonix linear actuator
// Jacob Anthony
// Feb. 2019

#define Enable    5
#define A         4
#define B         3
#define POS       A0

int pos;
int goal = 45;
int MAX = 0;

void setup() {
  pinMode(Enable, OUTPUT);
  pinMode(A,      OUTPUT);
  pinMode(B,      OUTPUT);

  pinMode(POS,    INPUT);

  digitalWrite(Enable,  LOW);
  digitalWrite(A,       LOW);
  digitalWrite(B,       LOW);
  
  Serial.begin(9600);
  
  pos = analogRead(POS);
  MAX = pos;

  Serial.println(pos);
  
  if(pos>75){
    while(pos>75){
      digitalWrite(A,       HIGH);
      digitalWrite(Enable,  HIGH);
      pos = analogRead(POS);
      Serial.println(pos);
    }
    digitalWrite(Enable,    LOW);
    digitalWrite(A,         LOW);
  }

}

void loop() {
  goal = 75;
  if(pos>goal){
    while(pos>goal){
      digitalWrite(A,       HIGH);
      digitalWrite(Enable,  HIGH);
      pos = analogRead(POS);
      Serial.println(pos);
    }
    digitalWrite(Enable,    LOW);
    digitalWrite(A,         LOW);
  }
  else if(pos<goal){
    while(pos<goal){
      digitalWrite(B,       HIGH);
      digitalWrite(Enable,  HIGH);
      pos = analogRead(POS);
      Serial.println(pos);
    }
    digitalWrite(Enable,    LOW);
    digitalWrite(B,         LOW);
  }

  delay(1000);

  goal = MAX;

  if(pos>goal){
    while(pos>goal){
      digitalWrite(A,       HIGH);
      digitalWrite(Enable,  HIGH);
      pos = analogRead(POS);
      Serial.println(pos);
    }
    digitalWrite(Enable,    LOW);
    digitalWrite(A,         LOW);
  }
  else if(pos<goal){
    while(pos<goal){
      digitalWrite(B,       HIGH);
      digitalWrite(Enable,  HIGH);
      pos = analogRead(POS);
      Serial.println(pos);
    }
    digitalWrite(Enable,    LOW);
    digitalWrite(B,         LOW);
  }

  delay(1000);

  goal = MAX/4;

  if(pos>goal){
    while(pos>goal){
      digitalWrite(A,       HIGH);
      digitalWrite(Enable,  HIGH);
      pos = analogRead(POS);
      Serial.println(pos);
    }
    digitalWrite(Enable,    LOW);
    digitalWrite(A,         LOW);
  }
  else if(pos<goal){
    while(pos<goal){
      digitalWrite(B,       HIGH);
      digitalWrite(Enable,  HIGH);
      pos = analogRead(POS);
      Serial.println(pos);
    }
    digitalWrite(Enable,    LOW);
    digitalWrite(B,         LOW);
  }

  delay(1000);

  goal = MAX*3/4;

  if(pos>goal){
    while(pos>goal){
      digitalWrite(A,       HIGH);
      digitalWrite(Enable,  HIGH);
      pos = analogRead(POS);
      Serial.println(pos);
    }
    digitalWrite(Enable,    LOW);
    digitalWrite(A,         LOW);
  }
  else if(pos<goal){
    while(pos<goal){
      digitalWrite(B,       HIGH);
      digitalWrite(Enable,  HIGH);
      pos = analogRead(POS);
      Serial.println(pos);
    }
    digitalWrite(Enable,    LOW);
    digitalWrite(B,         LOW);
  }

  delay(1000);

  goal = MAX/2;

  if(pos>goal){
    while(pos>goal){
      digitalWrite(A,       HIGH);
      digitalWrite(Enable,  HIGH);
      pos = analogRead(POS);
      Serial.println(pos);
    }
    digitalWrite(Enable,    LOW);
    digitalWrite(A,         LOW);
  }
  else if(pos<goal){
    while(pos<goal){
      digitalWrite(B,       HIGH);
      digitalWrite(Enable,  HIGH);
      pos = analogRead(POS);
      Serial.println(pos);
    }
    digitalWrite(Enable,    LOW);
    digitalWrite(B,         LOW);
  }

  delay(1000);

}
