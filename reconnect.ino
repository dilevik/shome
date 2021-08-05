void reconnect(){
 
 // while (!client.connected()) {
    Serial.println("Connecting to MQTT...");
 
    if (client.connect("ESP32Client")) {
 
      Serial.println("connected");
      client.subscribe("95/#");
     client.subscribe("h/#");
     // client.subscribe("/96/power/#");
       client.publish("esp95", "Hello from ESP32");
  //client.publish("h/to/add", "{\"name\": \"gdo\", \"service_name\": \"22\", \"service\": \"GarageDoorOpener\"}");
 
 
    } else {
 
      Serial.print("failed with state ");
      Serial.print(client.state());
     // delay(2000);
 
   // }
  }
 

  
  }
