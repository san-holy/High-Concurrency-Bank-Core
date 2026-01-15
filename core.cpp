#include<iostream>
#include<vector>
#include<string>
#include<thread>
#include<mutex>
#include<cstdlib>

class Account{
private:
    
public:
    int id;
    double balance;
    std::mutex accountmtx;//加锁之后，不可以使用vector<Account>,因为锁不可以被移动和复制
    Account(int id_,int balance_)
        :id(id_),balance(balance_) {}
};

class Bank{
    
public:

    std::vector<std::unique_ptr<Account>> accounts;//所以我们用指针
    Bank(int n,double initial_balance){
        for(int i=0;i<n;i++){
            //auto tmpptr=std::make_unique<Account>(i,initial_balance);
            //accounts.push_back(std::move(tmpptr));可以这样写

            accounts.push_back(std::move(std::make_unique<Account>(i,initial_balance)));
            //也可以这么写，直接生成一个指针并移动并添加
        }
    }

    void transfer(int from,int to,double amount){

        {
            accounts[from]->accountmtx.lock();
                accounts[to]->accountmtx.lock();
                
                accounts[from]->balance-=amount;
                accounts[to]->balance+=amount;
            
                accounts[to]->accountmtx.unlock();
            accounts[from]->accountmtx.lock();
        }

    }

    double getTotalBalance(){
        double sum=0;
    {
        for(int i=0;i<accounts.size();i++){
            //accounts[i]->accountmtx.lock();
            sum+=accounts[i]->balance;
            //accounts[i]->accountmtx.unlock();
        }
    }
        return sum;
    }
};
void manage(Bank& bank){
    for(int i=0;i<1000;i++){
        int from,to;
        do{
            from=rand()%100;
            to=rand()%100;
        }while(from==to);
        double amount=(double)(rand()%10+1);

        bank.transfer(from,to,amount);
    }
}
int main()
{
    srand(time(0));

    std::thread threads[10];
    Bank bank(100,1000.0);
    std::cout<<"initial:"<<bank.getTotalBalance()<<std::endl;

    for(int i=0;i<10;i++){
        threads[i]=std::thread(manage,std::ref(bank));
    }
    
    for(int i=0;i<10;i++){
        if(threads[i].joinable()) threads[i].join();
    }

    std::cout<<"finally:"<<bank.getTotalBalance()<<std::endl;
    return 0;
}