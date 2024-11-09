import numpy as np
import run_cython
import torch
import torch.nn as nn
from torch.autograd import Variable
import pygad
import os.path
#~ from torchsummary import summary

#~ print(torch.cuda.is_available())

input_size = 551
hidden_size = 100
hidden_size2 = 10
#~ hidden_size3 = 50
num_classes = 1
batch_size = 50
learning_rate = 0.1

class NeuralNet(nn.Module):
    def __init__(self):
        super(NeuralNet, self).__init__()

        torch.manual_seed(0)

        #~ self.conv1 = nn.Conv2d(1, 16, 3, 2, 1)
        #~ self.avgpool1 = nn.AvgPool2d(2, 2)
        #~ self.conv2 = nn.Conv2d(16, 32, 3, 2, 1)
        
        #~ self.tanh = nn.Tanh()
        
        #~ self.fc1 = nn.Linear(32 * 3 * 4, hidden_size)
        #~ self.fc2 = nn.Linear(hidden_size, hidden_size2)
        #~ self.fc3 = nn.Linear(hidden_size2, num_classes)

        self.fc1 = nn.Linear(input_size, hidden_size)
        self.act = nn.Sigmoid()
        #~ self.bn = nn.BatchNorm1d(hidden_size)
        self.fc2 = nn.Linear(hidden_size, hidden_size2)
        self.act2 = nn.Sigmoid()
        #~ self.bn2 = nn.BatchNorm1d(hidden_size2)
        self.fc3 = nn.Linear(hidden_size2, num_classes)
        #~ self.act3 = nn.Sigmoid()
        #~ self.fc4 = nn.Linear(hidden_size3, num_classes)


        #~ nn.init.xavier_uniform_(self.conv1.weight)
        #~ nn.init.xavier_uniform_(self.conv2.weight)
        nn.init.xavier_uniform_(self.fc1.weight)
        nn.init.xavier_uniform_(self.fc2.weight)
        nn.init.xavier_uniform_(self.fc3.weight)
        #~ nn.init.xavier_uniform_(self.fc4.weight)

        #~ nn.init.zeros_(self.conv1.bias)
        #~ nn.init.zeros_(self.conv2.bias)
        nn.init.zeros_(self.fc1.bias)
        nn.init.zeros_(self.fc2.bias)
        nn.init.zeros_(self.fc3.bias)
        #~ nn.init.zeros_(self.fc4.bias)

    def forward(self, x):
        #~ out = self.conv1(x)
        #~ out = self.tanh(out)
        #~ out = self.avgpool1(out)
        #~ out = self.conv2(out)
        #~ out = out.reshape(-1, 3 * 4 * 32)
        #~ out = self.fc1(out)
        #~ out = self.tanh(out)
        #~ out = self.fc2(out)
        #~ out = self.tanh(out)
        #~ out = self.fc3(out) 

        out = self.fc1(x)
        out = self.act(out)
        #~ out = self.bn(out)
        out = self.fc2(out)
        out = self.act2(out)
        #~ out = self.bn2(out)
        out = self.fc3(out)
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
batch_input = torch.zeros(dataset_size, 1, 1, 551)
#~ batch_input = torch.zeros(dataset_size, 1, 19, 29)
batch_label = torch.zeros(dataset_size, 1, 1, 1)

epochs = 500
train_batch_size = 2048

def fitness_func(ga_instance, solution, solution_idx):
    solution = solution > 0.5
    solution = np.concatenate((np.zeros(shape=(solution.shape[0], 1)), solution), axis=1)
    solution = np.concatenate((solution, np.zeros(shape=(solution.shape[0], 1))), axis=1)
    threads = 10
    val = solution.astype(np.bool)
    res = run_cython.py_compute(val, threads)
    #~ heatmap = res[:, 1:552].reshape(solution.shape[0], 1, 19, 29)
    
    global total_dataset_size
    
    select_batch = solution.shape[0]
    
    
    if((total_dataset_size + select_batch) >= dataset_size):
        select_batch = select_batch - ((total_dataset_size + select_batch) - dataset_size)
    
    inputs = torch.from_numpy(solution.reshape(solution.shape[0], 1, 1, 551).astype(np.float32))
    #~ inputs = torch.from_numpy(solution.reshape(solution.shape[0], 1, 19, 29).astype(np.float32))
    labels = torch.from_numpy(res[:, 0].reshape(solution.shape[0], 1, 1, 1).astype(np.float32))
    #~ print(f"s {select_batch} sol {solution.shape[0]}")
    batch_input[total_dataset_size:(total_dataset_size + select_batch)] = inputs[0:select_batch]
    batch_label[total_dataset_size:(total_dataset_size + select_batch)] = labels[0:select_batch]
    
    total_dataset_size = total_dataset_size + select_batch
    
    
    
    if(total_dataset_size >= dataset_size):
        
        #~ inputs = torch.from_numpy(solution.reshape(solution.shape[0], 1, 1, 551).astype(np.float32))
        #~ inputs = Variable(torch.from_numpy(heatmap.astype(np.float32)).cuda())
        #~ inputs = Variable(torch.from_numpy(solution.reshape(solution.shape[0], 1, 19, 29).astype(np.float32)).cuda())

        dataset = torch.utils.data.TensorDataset(batch_input, batch_label)
        dataloader = torch.utils.data.DataLoader(dataset, batch_size = train_batch_size, num_workers = 0)
        
        total_dataset_size = 0
        
        for epoch in range(1, epochs + 1):
            # monitor training loss
            trainLoss = 0.0
            for images, targets in dataloader:
                images, targets = Variable(images.cuda()), Variable(targets.cuda()) 
                optimizer.zero_grad()
                model.train()
                outputs = model(images)
                loss = criterion(outputs, targets)
                loss.backward()
                optimizer.step()
                trainLoss += loss.item() * images.size(0)
                  
            trainLoss = trainLoss / len(dataloader)
            
            inputs = batch_input[0:50]
            inputs = Variable(inputs.cuda()) 
            predicted_reward = predict(inputs, model).cpu()
            
            print(f"Epoch: {epoch:3d} Loss: {trainLoss:10.1f}    pred = {predicted_reward[0].item():7.0f} {predicted_reward[1].item():7.0f}    res = {batch_label[0][0][0][0]:7.0f} {batch_label[1][0][0][0]:7.0f}")
        
        #~ for i in range(30):
            #~ model.train()
            #~ outputs = model(inputs)
            #~ labels = Variable(torch.from_numpy(res[:, 0].reshape(solution.shape[0], 1).astype(np.float32)).cuda())
            #~ loss = criterion(outputs, labels) 
            #~ optimizer.zero_grad()
            #~ loss.backward()
            #~ optimizer.step() 
            #~ if(loss.item() < 10): break
                
        
        #~ if(res[0][0] < 10000):
            #~ print(f"out = {outputs}")
            #~ print(f"label = {labels}")
            #~ print(f"grad = {model.fc3.bias.grad}")

                
        #~ print(model.fc3.weight.grad)
        #~ print(model.fc3.weight)
        #~ print(f"{torch.min(model.fc1.weight.grad).item():5.2f} {torch.max(model.fc1.weight.grad).item():5.2f}  {torch.min(model.fc2.weight.grad).item():5.2f} {torch.max(model.fc2.weight.grad).item():5.2f}  {torch.min(model.fc3.weight.grad).item():5.2f} {torch.max(model.fc3.weight.grad).item():5.2f}")
        #~ print(f"{torch.min(model.fc1.weight).item():5.2f} {torch.max(model.fc1.weight).item():5.2f}  {torch.min(model.fc2.weight).item():5.2f} {torch.max(model.fc2.weight).item():5.2f}  {torch.min(model.fc3.weight.grad).item():5.2f} {torch.max(model.fc3.weight).item():5.2f}")
        
        #~ if(torch.isnan(model.fc1.weight.grad).any() or torch.isnan(model.fc1.bias.grad).any()):
            #~ printf("!!!!!!!!!!!!!")
        #~ if(torch.isnan(model.fc2.weight.grad).any() or torch.isnan(model.fc2.bias.grad).any()):
            #~ printf("!!!!!!!!!!!!!")
        #~ if(torch.isnan(model.fc3.weight.grad).any() or torch.isnan(model.fc3.bias.grad).any()):
            #~ printf("!!!!!!!!!!!!!")
        
        #~ predicted_reward = predict(inputs, model).cpu()
        
        #~ global total_fitness_cnt
        #~ total_fitness_cnt = total_fitness_cnt + 1
        #~ if(total_fitness_cnt % 5 == 0):
        #~ print(f"Loss: {loss.item():10.1f}    pred = {predicted_reward[0].item():7.0f} {predicted_reward[1].item():7.0f}    res = {res[0][0]:7d} {res[1][0]:7d}")
        
    #~ print(res.shape)
    #~ print(f"res = {res[0][0]} {res[1][0]}")
    return res[:, 0]
    
    
num_generations = 1000000000000 # Number of generations.
num_parents_mating = 10 # Number of solutions to be selected as parents in the mating pool.

sol_per_pop = 1000 # Number of solutions in the population.
num_genes = 549

last_fitness = 0
def on_generation(ga_instance):
    global last_fitness
    print(f"{ga_instance.generations_completed:10d} F = {ga_instance.best_solution(pop_fitness=ga_instance.last_generation_fitness)[1]}")
    last_fitness = ga_instance.best_solution(pop_fitness=ga_instance.last_generation_fitness)[1]
    
    if ga_instance.generations_completed % 50 == 0:
        ga_instance.save("pygad")
        #~ torch.save(model, "predict_reward.ckp")
        torch.save({
            'model_state_dict': model.state_dict(),
            'optimizer_state_dict': optimizer.state_dict(),
            }, "predict_reward.ckp")
    
    
if(os.path.isfile("predict_reward.ckp")):
    #~ model = torch.load("predict_reward.ckp")
    checkpoint = torch.load("predict_reward.ckp")
    model.load_state_dict(checkpoint['model_state_dict'])
    optimizer.load_state_dict(checkpoint['optimizer_state_dict'])
    
if(os.path.isfile("pygad.pkl")):
    ga_instance = pygad.load("pygad")
    ga_instance.on_generation=on_generation
    ga_instance.fitness_func=fitness_func
    ga_instance.fitness_batch_size=batch_size
else:
    ga_instance = pygad.GA(num_generations=num_generations,
                       num_parents_mating=num_parents_mating,
                       #~ gene_type = int,
                       fitness_batch_size = batch_size,
                       sol_per_pop=sol_per_pop,
                       num_genes=num_genes,
                       fitness_func=fitness_func,
                       init_range_low=0.0,
                       init_range_high=1.0,
                       #~ mutation_percent_genes=10.0,
                       #~ mutation_type="random",
                       mutation_percent_genes=[0.1, 1],
                       mutation_type="adaptive",
                       #~ mutation_type="inversion",
                       mutation_by_replacement=True,
                       random_mutation_min_val=0.0,
                       random_mutation_max_val=1.0,
                       crossover_type="single_point",
                       on_generation=on_generation)
                       

# Running the GA to optimize the parameters of the function.
ga_instance.run()
