void saveMsgPos(const char * msg)
{
  File f = SPIFFS.open("/file.txt", "a");
  f.print(msg);
  f.print("\r");
  f.close();
}

int qtdeMsgPos()
{
  File f = SPIFFS.open("/file.txt", "r");
  int qtde = 0;
  String content = f.readStringUntil('\r');
  do
  {
    if (content != "") qtde++;
  } while ((content = f.readStringUntil('\r')) != "");
  f.close();
  return qtde;
}

String readMsgPos()
{
  File f = SPIFFS.open("/file.txt", "r");
  String content = f.readStringUntil('\r');
  String line = f.readStringUntil('\r');
  String out;
  do
  {
    if (line != "") { out += line; out += "\r"; }
  } while ((line = f.readStringUntil('\r')) != "");
  f.close();

  File f2 = SPIFFS.open("/file.txt", "w");
  f2.print(out);
  f2.close();
  
  return content;
}
