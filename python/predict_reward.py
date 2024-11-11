import numpy as np
import run_cython
import torch
import torch.nn as nn
from torch.autograd import Variable
import os.path
#~ from torchsummary import summary

#~ print(torch.cuda.is_available())

input_size = 551
hidden_size = 100
hidden_size2 = 50
#~ hidden_size3 = 50
num_classes = 1

batch_size = 50

learning_rate = 0.001
#~ learning_rate = 0.1
#~ learning_rate = 10.0

class NeuralNet(nn.Module):
    def __init__(self):
        super(NeuralNet, self).__init__()

        torch.manual_seed(0)

        self.conv1 = nn.Conv2d(1, 64, 3, 1, 1)
        self.ln1 = nn.LayerNorm([64, 19, 29])
        self.conv2 = nn.Conv2d(64, 128, 3, 1, 1)
        self.ln2 = nn.LayerNorm([128, 19, 29])
        #~ self.avgpool1 = nn.AvgPool2d(2, 2)
        self.conv3 = nn.Conv2d(128, 256, 3, 2, 1)
        self.ln3 = nn.LayerNorm([256, 10, 15])
        self.conv4 = nn.Conv2d(256, 512, 3, 2, 1)
        self.ln4 = nn.LayerNorm([512, 5, 8])
        
        self.conv5 = nn.Conv2d(128, 256, 1, 2, 0)
        self.conv6 = nn.Conv2d(256, 512, 1, 2, 0)
        
        self.act = nn.ReLU()
        
        self.fc1 = nn.Linear(512 * 5 * 8, hidden_size)
        self.fc2 = nn.Linear(hidden_size, hidden_size2)
        self.fc3 = nn.Linear(hidden_size2, num_classes)

        #~ self.fc1 = nn.Linear(input_size, hidden_size)
        #~ self.act = nn.ReLU()
        #~ self.bn = nn.BatchNorm1d(hidden_size)
        #~ self.fc2 = nn.Linear(hidden_size, num_classes)
        #~ self.act2 = nn.Sigmoid()
        #~ self.bn2 = nn.BatchNorm1d(hidden_size2)
        #~ self.fc3 = nn.Linear(hidden_size2, num_classes)
        #~ self.act3 = nn.Sigmoid()
        #~ self.fc4 = nn.Linear(hidden_size3, num_classes)


        nn.init.xavier_uniform_(self.conv1.weight)
        nn.init.xavier_uniform_(self.conv2.weight)
        nn.init.xavier_uniform_(self.conv3.weight)
        nn.init.xavier_uniform_(self.conv4.weight)
        nn.init.xavier_uniform_(self.conv5.weight)
        nn.init.xavier_uniform_(self.conv6.weight)
        nn.init.xavier_uniform_(self.fc1.weight)
        nn.init.xavier_uniform_(self.fc2.weight)
        nn.init.xavier_uniform_(self.fc3.weight)
        #~ nn.init.xavier_uniform_(self.fc4.weight)
        #~ nn.init.constant_(self.bn1.weight, 1)
        #~ nn.init.constant_(self.bn2.weight, 1)
        #~ nn.init.constant_(self.bn3.weight, 1)
        #~ nn.init.constant_(self.bn4.weight, 1)

        nn.init.zeros_(self.conv1.bias)
        nn.init.zeros_(self.conv2.bias)
        nn.init.zeros_(self.conv3.bias)
        nn.init.zeros_(self.conv4.bias)
        nn.init.zeros_(self.conv5.bias)
        nn.init.zeros_(self.conv6.bias)
        nn.init.zeros_(self.fc1.bias)
        nn.init.zeros_(self.fc2.bias)
        nn.init.zeros_(self.fc3.bias)
        #~ nn.init.zeros_(self.fc4.bias)
        #~ nn.init.constant_(self.bn1.bias, 0)
        #~ nn.init.constant_(self.bn2.bias, 0)
        #~ nn.init.constant_(self.bn3.bias, 0)
        #~ nn.init.constant_(self.bn4.bias, 0)

    def forward(self, x):
        out = self.conv1(x)
        out = self.ln1(out)
        out = self.act(out)
        out = self.conv2(out)
        out = self.ln2(out)
        out = self.act(out)
        out = out + x
        
        #~ print(out.shape)
        
        res = out
        out = self.conv3(out)
        out = self.ln3(out)
        out = self.act(out)
        out = self.conv4(out)
        out = self.ln4(out)
        out = self.act(out)
        out = out + self.conv6(self.conv5(res))
        
        #~ print(out.shape)
        
        #~ out = self.avgpool1(out)
        #~ out = self.conv2(out)
        #~ out = self.bn2(out)
        #~ out = self.act(out)
        out = out.reshape(-1, 5 * 8 * 512)
        out = self.fc1(out)
        out = self.act(out)
        out = self.fc2(out)
        out = self.act(out)
        out = self.fc3(out)
        out = self.act(out)#relu



        #~ out = self.fc1(x)
        #~ out = self.act(out)
        #~ out = self.bn(out)
        #~ out = self.fc2(out)
        #~ out = self.act2(out)
        #~ out = self.bn2(out)
        #~ out = self.fc3(out)
        #~ out = self.act3(out)
        #~ out = self.fc4(out)
        
        #~ print(out.shape)
        return out

def predict(data_batch, model):
    model.eval() 
    outputs = model(data_batch)
    return outputs

    
model = NeuralNet()
model.cuda()
#~ summary(model, (1, 19, 29))
#~ summary(model, (1, 1, 551))

criterion = nn.L1Loss()
#~ criterion = nn.MSELoss()
#~ optimizer = torch.optim.SGD(model.parameters(), lr=learning_rate) 
optimizer = torch.optim.Adam(model.parameters(), lr=learning_rate, weight_decay=1e-5) 

#~ total_fitness_cnt = 0

total_dataset_size = 0

dataset_size = 10000
#~ batch_input = torch.zeros(dataset_size, 1, 1, 551)
batch_input = torch.zeros(dataset_size, 1, 19, 29)
#~ batch_label = torch.zeros(dataset_size, 1, 1, 1)
batch_label = torch.zeros(dataset_size, 1)

epochs = 200
train_batch_size = 512
#~ train_batch_size = 10000

run_training = 1
#run_training = 0


threads = 10

run_cython.py_load_state()
pop = run_cython.py_get_population()

res = run_cython.py_compute(pop.astype(np.bool), threads)

while(1):

    run_cython.py_gen_population(res[:, 0].copy())
    pop = run_cython.py_get_population()
    
    res = run_cython.py_compute(pop.astype(np.bool), threads)
    
    
    if(run_training):
        select_batch = pop.shape[0]
        
        
        if((total_dataset_size + select_batch) >= dataset_size):
            select_batch = select_batch - ((total_dataset_size + select_batch) - dataset_size)
        
        #~ inputs = torch.from_numpy(pop.reshape(pop.shape[0], 1, 1, 551).astype(np.float32))
        inputs = torch.from_numpy(pop.reshape(pop.shape[0], 1, 19, 29).astype(np.float32))
        #~ labels = torch.from_numpy(res[:, 0].reshape(pop.shape[0], 1, 1, 1).astype(np.float32))
        labels = torch.from_numpy(res[:, 0].reshape(pop.shape[0], 1).astype(np.float32))

        batch_input[total_dataset_size:(total_dataset_size + select_batch)] = inputs[0:select_batch]
        batch_label[total_dataset_size:(total_dataset_size + select_batch)] = labels[0:select_batch]
        
        total_dataset_size = total_dataset_size + select_batch
        
        
        
        if(total_dataset_size >= dataset_size):

            dataset = torch.utils.data.TensorDataset(batch_input, batch_label)
            dataloader = torch.utils.data.DataLoader(dataset, batch_size = train_batch_size, num_workers = 0, shuffle=True)
            
            total_dataset_size = 0
            
            for epoch in range(1, epochs + 1):
            #~ epoch = 1
            #~ while(1):
                # monitor training loss
                trainLoss = 0.0
                for images, targets in dataloader:
                    images, targets = Variable(images.cuda()), Variable(targets.cuda()) 
                    optimizer.zero_grad()
                    model.train()
                    outputs = model(images)
                    loss = criterion(torch.log(outputs + 1), torch.log(targets + 1))
                    loss.backward()
                    optimizer.step()
                    trainLoss += loss.item() * images.size(0)
                      
                trainLoss = trainLoss / len(dataloader)
                
                inputs = batch_input[0:50]
                inputs = Variable(inputs.cuda()) 
                predicted_reward = predict(inputs, model).cpu()
                
                #~ print(f"Epoch: {epoch:3d} Loss: {trainLoss:10.1f}    pred = {predicted_reward[0].item():7.0f} {predicted_reward[1].item():7.0f}    res = {batch_label[0][0][0][0]:7.0f} {batch_label[1][0][0][0]:7.0f}")
                #~ print(f"Epoch: {epoch:3d} Loss: {trainLoss:10.1f}    pred = {predicted_reward[0].item():7.0f} {predicted_reward[1].item():7.0f}    res = {batch_label[0][0]:7.0f} {batch_label[1][0]:7.0f}")
                print(f"Epoch: {epoch:3d} Loss: {trainLoss:10.1f}    pred = {predicted_reward[0].item():7.0f} {predicted_reward[1].item():7.0f}    res = {batch_label[0][0]:7.0f} {batch_label[1][0]:7.0f}")
                #~ epoch = epoch + 1
                
                if(trainLoss < 50.0): 
                    break
            
            torch.save({'model_state_dict': model.state_dict(), 'optimizer_state_dict': optimizer.state_dict()}, "predict_reward.ckp")
            
