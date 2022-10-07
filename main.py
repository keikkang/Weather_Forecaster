import ui
import db

app = ui.QApplication(ui.sys.argv)
sjlab_forecast = ui.User_interface()    
sjlab_forecast.setup_btn()
sjlab_forecast.show()
app.exec_() 


