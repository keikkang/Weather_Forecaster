from pickle import NONE
import sys
from PyQt5.QtWidgets import *
from PyQt5 import uic
from dataclasses import dataclass
import db
import ml
import torch
import torch.nn as nn
import torch.nn.functional as F

class UiStatus:
    crawling_pressed : bool = False
    learning_pressed : bool = False
    connecting_pressed : bool = False
    request_pressed : bool = False
    prediction_pressed : bool = False
    month_combo : str = None
    year_combo : str = None
    ml_combo : str = None
    ip_combo : str =  None
    port_combo : str = None
    db_combo : str = None
    

weather_ui = uic.loadUiType("weather_forecast.ui")[0]

class User_interface(QMainWindow, weather_ui):
    def __init__(self):
        super().__init__()
        self.setupUi(self)
        self.ui = UiStatus()
        self.sjlab_db = db.Sj_Sql(**db.SQL_INFO)
    
    def setup_btn(self):
        """initalize btn"""
        self.crawling_btn.clicked.connect(self.crawling_btn_clicked)
        self.learning_btn.clicked.connect(self.learning_btn_clicked)
        self.prediction_btn.clicked.connect(self.prediction_btn_clicked)
        self.request_btn.clicked.connect(self.request_btn_clicked)
        self.clear_btn.clicked.connect(self.clear_btn_clicked)
    
    def crawling_btn_clicked(self):
        """crawling btn"""
        self.command_browser.append("Crawling Start...")
        self.weather = db.Web_Crawling(5) #7month data extract 
        self.weather.get_webpage()
        self.weather.find_table()
        self.weather.find_span()
        self.crawl_list = self.weather.find_str_index()
        #print(self.weather.cal_mean_month_temp())
        self.command_browser.append(str(self.crawl_list))
        ml.ML_DB['x_train'] = self.crawl_list
        ml.ML_DB['y_train'] = ml.test_y_train
        self.command_browser.append("Crawling End...")
        self.ui.crawling_pressed = True
        
    def learning_btn_clicked(self):
        """learning btn"""
        self.command_browser.append("Learning Start...")
        if not(self.ui.request_pressed or self.ui.crawling_pressed):
            self.command_browser.append("Must crawling data and request data first...")
        ml.do_cal()
        self.command_browser.append("Learning End...")
        
        #self.ml_class.set_train_data(self.crawl_list,self.)
    
    def request_btn_clicked(self):
        """request btn """
        self.command_browser.append("Request Start...")
        self.sjlab_db.connect()
        self.db_temp_data = self.sjlab_db.select_all(db.SQL_SELECT_TEMP)
        print(self.db_temp_data)
        self.command_browser.append("Request End...")
        self.ui.request_pressed = True
        
    def prediction_btn_clicked(self):
        #QMessageBox.about(self, "message", "Prdiciton_status")
        self.command_browser.append("Prediction Start...")
        if(self.line_edit.text()):
            new_val = self.line_edit.text()
            print(type(new_val))
            new_val = (float)(new_val)
            new_val = torch.FloatTensor([[new_val]])
            pred_val =ml.model(new_val)
            print(pred_val)
            self.command_browser.append("예상온도 : %f"%pred_val)
            self.command_browser.append("Request End...")
    
    def clear_btn_clicked(self):
        self.command_browser.clear()
        
    def update_from_db(self):
        pass
    
    def get_ui_status(self):
        return self.ui_packet
      

#sjlab_db = db.Sj_Sql(**db.SQL_INFO)
#sjlab_db.connect()
#temp_data = sjlab_db.select_all(db.SQL_SELECT_TEMP)

