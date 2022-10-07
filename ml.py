import torch
import torch.nn as nn
import torch.nn.functional as F
import ui


ML_DB = {'x_train':[], 'y_train':[]}
test_y_train = [22.1, 23.3 , 24.2 , 28.7, 30.35, 31.4 , 29.6, 24.95, 28.85, 31.85, 31.55, 33.35, 31.1, 25.55, 
                26.3, 27.95, 32, 32, 31.85, 32.9, 32.15, 32.45, 36.5, 36.35, 33.5, 30.05, 32, 35.45, 36.65, 33.2, 33.05]

class Liner_Regression(nn.Module):
    def __init__(self):
        super().__init__()
        self.linear = nn.Linear(1,1)
        #self.nb_epohcs = 2000
    
    def forward(self, x):
        return self.linear(x)
    
    def input_train_data(self, **kwargs):
        
        self.int_x_list = list(map(float,kwargs['x_train']))
        self.int_y_list = list(map(float,kwargs['y_train']))
        
        self.x_train = torch.FloatTensor(self.int_x_list)
        self.x_train = self.x_train.unsqueeze(1)
        self.y_train = torch.FloatTensor(self.int_y_list)
        self.y_train = self.y_train.unsqueeze(1)
        
        print(self.x_train)
        print(self.y_train)        
        print(self.x_train.shape)
        print(self.y_train.shape)
        
def do_cal():
    global model 
    model = Liner_Regression()
    model.input_train_data(**ML_DB)

    optimizer = torch.optim.SGD(model.parameters(), lr=0.001) 

    nb_epochs = 20000
    
    for epoch in range(nb_epochs+1):
        # H(x) 계산
        prediction = model(model.x_train) 

        # cost 계산
        cost = F.mse_loss(prediction, model.y_train) # <== 파이토치에서 제공하는 평균 제곱 오차 함수

        # cost로 H(x) 개선하는 부분
        # gradient를 0으로 초기화
        optimizer.zero_grad()
        # 비용 함수를 미분하여 gradient 계산
        cost.backward() # backward 연산
        # W와 b를 업데이트
        optimizer.step()

        if epoch % 100 == 0:
            # 100번마다 로그 출력
            print('Epoch {:4d}/{} Cost: {:.6f}'.format(
                epoch, nb_epochs, cost.item()))
    