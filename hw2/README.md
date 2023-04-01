# HW-2 Socket Programming and Data Visualization

[GitHub repo](https://github.com/Leng-Kai/EE3021-hw)

## Design

### 1. Mbed Studio

參考範例 code 來修改 mbed-os-example-sockets 的 main.cpp，在 `_socket.connect(address)` 建立完 connection 後將 sensor 得到的值透過 sprintf 寫入字串再透過 socket 送出，並用 `ThisThread::sleep_for()` 來讓 while 迴圈隔一秒重複執行： 

```cpp
// Line 132
        int sample_num = 0;
        char acc_json[1024];
#define SCALE_MULTIPLIER 1
        while (1) {
            ++sample_num;
            BSP_ACCELERO_AccGetXYZ(pDataXYZ);
            printf("\nACCELERO_X = %d\n", pDataXYZ[0]);
            printf("ACCELERO_Y = %d\n", pDataXYZ[1]);
            printf("ACCELERO_Z = %d\n", pDataXYZ[2]);
            int x = pDataXYZ[0]*SCALE_MULTIPLIER, y = pDataXYZ[1]*SCALE_MULTIPLIER, z = pDataXYZ[2]*SCALE_MULTIPLIER;
           // int len = sprintf(acc_json,"{\"x\":%d,\"y\":%d,\"z\":%d,\"s\":%d}",(float)((int)(x*10000))/10000, (float)((int)(y*10000))/10000, (float)((int)(z*10000))/10000, sample_num);
            int len = sprintf(acc_json,"{\"x\":%d,\"y\":%d,\"z\":%d,\"s\":%d}", x, y, z, sample_num);
            printf("payload: %s\n", acc_json);

            int response = _socket.send(acc_json,len);
            if (0 >= response){
                printf("Error seding: %d\n", response);
            }
            ThisThread::sleep_for(1000);
        }
```

在 Mbed Studio 的 console 可以看到 STM32 在成功連上 WiFi 後每秒傳送的 payload 內容：

![](https://cdn.discordapp.com/attachments/1085939874328232086/1091646709999878144/image.png)

在另一台機器上跑 Python socket server 以接收 STM32 透過 socket 傳送的 payload，並使用 matplotlib 將結果視覺話，將圖表輸出成 plot.pdf：

```python
# Line 13
    with conn:
		print("Connected at", addr)
		while True:
			data = conn.recv(1024).decode()
			print("Received from socket server:", data)
			if (data.count('{') != 1):
				# Incomplete data are received.
				choose = 0
				buffer_data = data.split('}')
				while buffer_data[choose][0] != '{':
					choose += 1
				data = buffer_data[choose] + '}'
				
			obj = json.loads(data)

			t = obj['s']
			plot.scatter(t, obj['x'], c='blue') # x, y, z, gx, gy, gz
			plot.xlabel("sample num")
			plot.pause(0.0001)
			plot.savefig('plot.pdf')
```

在執行 socket-server.py 後可以看到 server 接收到 STM32 的 connection：

![](https://cdn.discordapp.com/attachments/1085939874328232086/1091646941940682822/image.png)

輸出的圖表 plot.pdf：

![](https://i.imgur.com/c5J3ZLQ.png)


### 2. STM32CubeIDE

修改 CubeIDE 裡的 WiFi_Client_Server 範例程式，在使用 `WIFI_OpenClientConnection()` 連上 Python socket server 後開始收集 sensor 資料並透過 socket 送出：

```c
// Line 189
  BSP_ACCELERO_Init();

  while(1)
  {
	int16_t pDataXYZ[3] = {0};
	int sample_num = 0;
	char acc_json[1024];
#define SCALE_MULTIPLIER 1
    while (1) {
    	++sample_num;
    	BSP_ACCELERO_AccGetXYZ(pDataXYZ);
    	TERMOUT("\nACCELERO_X = %d\n", pDataXYZ[0]);
    	TERMOUT("ACCELERO_Y = %d\n", pDataXYZ[1]);
    	TERMOUT("ACCELERO_Z = %d\n", pDataXYZ[2]);
        int x = pDataXYZ[0]*SCALE_MULTIPLIER, y = pDataXYZ[1]*SCALE_MULTIPLIER, z = pDataXYZ[2]*SCALE_MULTIPLIER;
        int len = sprintf(acc_json,"{\"x\":%d,\"y\":%d,\"z\":%d,\"s\":%d}", x, y, z, sample_num);
        TERMOUT("payload: %s\n", acc_json);

        WIFI_SendData(Socket, acc_json, len, &Datalen, WIFI_WRITE_TIMEOUT);

        HAL_Delay(1000);
    }
```

作法與 Mbed Studio 相同，只需將 socket 相關函數做相對應的調整，以及改成使用 `TERMOUT()`、`HAL_Delay()` 等函數。

由於傳送 payload 格式與 Mbed Studio 作法相同，可以使用相同的 `socket-server.py` 來作為 socket server。


## Review questions

### 1. What are the units used for the sensor values?

### 2. What is I2C read address and I2C write address allocated for the LSM6DSL 3D accelerometer and 3D 

### 3. What are the main differences I2C between SMbus (System Management Bus)?

I2C 的傳輸速度較快，addressing space 也較大，且 I2C 支援 clock stretching。

### 4. What is the I2C address of ADXL 345, if ALT ADDRESS is connected to HIGH?

0x17，根據 ADXL 345 manual：

![](https://i.imgur.com/kFs4Eg0.png)

### 5. How to connect two signal lines to achieve the wired-AND logic?

Lecture slides 中提到的 I2C bus driver 的 open drain 特性：
- meaning that they can pull the corresponding signal line low, but cannot drive it high. -> Wired-AND logic
- Each signal line has a common pull-up resistor on it, to restore the signal to high when no device is asserting it low

只要有任何一個 component 輸出 low，bus 上的狀態就會是 low，必須要所有 component 都輸出 high，bus 上的狀態才會是 high。

### 6. What is the main difference between the bus master and the bus slave?

master 通常是能主動發起 transaction 的 component，例如 CPU、DMA controller 等。slave 通常無法主動發起 transation，只能被動等待 master 發起的 transaction 並作出回應，例如 ram、I/O device 等。
