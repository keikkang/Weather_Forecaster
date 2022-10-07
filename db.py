#from codecs import utf_8_encode
import pymysql #mysql
import requests #for weather forcase
from bs4 import BeautifulSoup #HTML parsing
#import ui


SQL_INFO = {'host':'192.68.20.20', 'port':3306, 'user':'kks','password':'kskang123!','database':'iot_project','charset':'utf8'}
SQL_SELECT_TEMP = "select temp from sensor_table"
SQL_SELECT_TIME = "select regtime from sensor_table"



class Sj_Sql():
    def __init__(self, **args):
        self.host = args['host']
        self.port = args['port']
        self.user = args['user']
        self.password = args['password']
        self.database = args['database']
        self.charset = args['charset']
        print(self.host)
        
    def connect(self):
        """Connect Linux Server MYSQL"""
        self.con =pymysql.Connect(host=self.host ,port = self.port, user=self.user, password=self.password, database=self.database, charset=self.charset)
        self.cur = self.con.cursor()
            
    def select_one(self, sql, args = None):
        """Select one data from DB"""
        self.cur.execute(sql, args)
        result = self.cur.fetchone()
        self.cur.close()
        self.con.close()
        return result
    
    def select_all(self, sql, args= None):
        """Select all data from DB"""
        self.cur.execute(sql,args)
        result = self.cur.fetchall()
        self.cur.close()
        self.con.close()
        return result

class Web_Crawling():
    def __init__(self,month):
        """Make Target URL"""
        self.url = 'https://www.weather.go.kr/w/obs-climate/land/past-obs/obs-by-day.do?stn=108&yy=2022&mm={}&obs=1'.format(month)     
    def get_webpage(self):
        self.response = requests.get(self.url)
        self.soup = BeautifulSoup(self.response.content, 'html.parser')
    def find_table(self):
        """find target str for crawling"""
        self.table = self.soup.find('table',{'class': 'table-col table-cal'}) 
        #print(self.table)
    def find_span(self):
        """find specific span"""
        self.text_datas = []
        self.none_space_string = []
        self.td = []
        self.td = self.table.find_all('td')
        for self.span in self.td:
            self.text_datas.append(self.span.get_text())
        print(self.text_datas)   
        print(type(self.text_datas))
        for text_data in self.text_datas:
            self.none_space_string.append(''.join(text_data.split())) #erase string empty place
        print(self.none_space_string)
        #print(sys.getdefaultencoding())
                        
    def find_str_index(self):
        self.string = 0
        self.indexs = 0
        self.db_list = []
        #while self.index < len(self.none_space_string):
        for self.string in self.none_space_string:
            self.indexs = self.string.find('평균기온')
            print(self.indexs)
            if self.indexs != -1:
                self.db_list.append(self.string[5:9])
        print(type(self.db_list))
        return self.db_list
        
    def cal_mean_month_temp(self):
        self.float_db_list = []
        self.mean_temp = None
        for i in self.db_list:
            self.float_db_list = float(i)
        self.mean_temp = self.float_db_list/len(self.db_list)
        return self.mean_temp