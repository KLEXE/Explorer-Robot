#include <SPI.h>
#include <WiFi.h>
#include <HttpClient.h>
#include <Xively.h>

char ssid[] = "";   //  WiFi SSID
char pass[] = "";    // Password for WiFi SSID
int keyIndex = 0;            // your network key Index number (needed only for WEP)

int status = WL_IDLE_STATUS;

// To API Key του Xively προκειμένου να στείλουμε δεδομένα
char xivelyKey[] = "It";
//Χively feed ID
#define xivelyFeed 5
//datastreams (ονομάτα καναλιών του xively
char PhID[] = "Ph";
char TempID[] = "Temperature";

// Pin για θερμοκρασία και για Ph
#define PhPin A0
#define TempPin A3


// Define the strings for our datastream IDs
XivelyDatastream datastreams[] = {
  XivelyDatastream(PhID, strlen(PhID), DATASTREAM_FLOAT),
  XivelyDatastream(TempID, strlen(TempID), DATASTREAM_FLOAT),
};
// Finally, wrap the datastreams into a feed
XivelyFeed feed(xivelyFeed, datastreams, 2 /* number of datastreams */);

WiFiClient client;
XivelyClient xivelyclient(client);

void printWifiStatus() {
  // To SSID του δικτύου
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  // Η διεύθυνση IP που πήρε η WiFi Shield
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);

  // Η δύναμη του σήματος
  long rssi = WiFi.RSSI();
  Serial.print("signal strength (RSSI):");
  Serial.print(rssi);
  Serial.println(" dBm \n");
}
//Η διαδικασία που καλείται για να επιτευχθεί η σύνδεση με το WiFi δίκτυο
void connectWiFi()
{
  // Δεν αν υπάρχει σύνδεση με την WiFi shield
  if (WiFi.status() == WL_NO_SHIELD)
  {
    Serial.println("[ERROR] WiFi Shield Not Present");
    // Do not continue
    while (true);
  }

  // Προσπάθει να γίνει σύνδεση
  while ( status != WL_CONNECTED)
  {
    Serial.print("Attempting Connection - WPA SSID:");
    Serial.println(ssid);

    status = WiFi.begin(ssid, pass);
  }

  // Connection successful
  Serial.print(" Connection Successful");
  Serial.print("");
  Serial.println("-----------------------------------------------");
  Serial.println("");
}
void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  //pin setup
  pinMode(PhPin, INPUT);
  pinMode(TempPin, INPUT);
  pinMode(13, OUTPUT);
  Serial.println("Starting single datastream upload to Xively...");
  Serial.println();

  connectWiFi();
  printWifiStatus();
}


void loop() {
  int getReturn = xivelyclient.get(feed, xivelyKey);    //Επικοινωνία με την πλατφόρμα xively
  if (getReturn > 0) {
    Serial.println("Ph Datastream");
    Serial.println(feed[0]);
  } else Serial.println("HTTP Error");

  //Για την μέτρηση του Ph
  unsigned long int avgValue;  //Για την μέση τιμή του αισθητήρα
  float b;
  int buf[10], temp;
  for (int i = 0; i < 10; i++) //Λήψη 10 δειγμάτων για πιο σωστή μέτρηση
  {
    buf[i] = analogRead(PhPin);
    delay(10);
  }
  for (int i = 0; i < 9; i++) //Ταξινόμηση των δειγμάτων από την μικρότερη προς την μεγαλύτερη
  {
    for (int j = i + 1; j < 10; j++)
    {
      if (buf[i] > buf[j])
      {
        temp = buf[i];
        buf[i] = buf[j];
        buf[j] = temp;
      }
    }
  }
  avgValue = 0;
  for (int i = 2; i < 8; i++)               //Λήψη της μέσης τιμής από τα 6 δείγματα
    avgValue += buf[i];
  float phValue = (float)avgValue * 5.0 / 1024 / 6; //μετατροπή της μέτρησης σε milivolt
  phValue = 3.5 * phValue;

  //Για την μέτρηση της θερμοκρασίας
  int tempReading = analogRead(TempPin);
  // Μετατροπή σε τάση
  float voltage = tempReading * 5 / 1024.0;
  // Μετατροπή σε θερμοκρασία
  float celsius = (voltage - 0.5) * 100 ;

  datastreams[0].setFloat(phValue);
  datastreams[1].setFloat(celsius);

  //Εκτύπωση των τιμών
  Serial.print("Read sensor value ");
  Serial.println(datastreams[0].getFloat());
  Serial.println(datastreams[1].getFloat());

  //send value to xively
  Serial.println("Uploading it to Xively");
  int ret = xivelyclient.put(feed, xivelyKey);
  //return message
  Serial.print("xivelyclient.put returned ");
  Serial.println(ret);
  Serial.println("");


  //delay between calls
  delay(15000);
}
