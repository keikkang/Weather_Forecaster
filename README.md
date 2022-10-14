# Temperatur_Forecaster

## Overview
This is a small project to determine the correlation between local temperature and the indoor temperature of the lab where I work.

1. Request temperature data stored in local Liux Server MY SQL db.
2. Parse the temperature data from the South Korea Meteorological Agency html.
3. The ML model analyzes the correlation between the laboratory temperature and the average temperature of the Korea Meteorological Administration.
4. When you enter a temperature, it tells you the expected lab temperature.
5. A series of processes can be carried out directly by the user through the UI.

## System Block
![image](https://user-images.githubusercontent.com/108905975/195729597-fdd137d8-cfff-4678-b00b-fb566a57f25a.png)

![image](https://user-images.githubusercontent.com/108905975/194469863-69f9e8f5-66f7-4506-93ac-0b832911d7ab.png)

## User Interface
![image](https://user-images.githubusercontent.com/108905975/194469951-4504ad16-5818-498f-89c1-968237857232.png)

## Development environment
### Server
|Name|Description|Note|
|:------|:---|:---|
|Mini_IoT_Platform|Local Network Server|[Link]|
### Python Library(Ver. 3.10.5)
|Name|Description|Note|
|:---|:---|:---|
|PyQt|User Interface|Ver. 5.15.7|
|torch|Machine Learning|Ver. 1.12.1|
|PyMySQL|Data Base Request|Ver. 1.0.2|
|BeautifulSoup4|Web html Parsing|Ver. 4.11.1|

## Settings
```
pip install torch
pip install PyMySQL
pip install BeautifulSoup4
pip install PyQt
```
You can modify values for your servere
db.py
```python
SQL_INFO = {'host':'Your Host', 'port':Your port, 'user':'Your SQL User name','password':'Your SQL PW','database':'Your DB Name','charset':'utf8'}
```
