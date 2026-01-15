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
    Account(int id_,int balance_)
        :id(id_),balance(balance_) {}
};

class Bank{
    std::vector<Account> accounts;
public:
    Bank(int n,double initial_balance){
        for(int i=0;i<n;i++){
            accounts.push_back(Account(i,initial_balance));
        }
    }

    void transfer(int from,int to,double amount){
        accounts[from].balance-=amount;
        accounts[to].balance+=amount;
    }

    double getTotalBalance(){
        double sum=0;
        for(int i=0;i<accounts.size();i++){
            sum+=accounts[i].balance;
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