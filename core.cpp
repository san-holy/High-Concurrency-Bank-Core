#include<iostream>
#include<vector>
#include<string>
#include<thread>
#include<mutex>

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
    Bank(int n,double *initial_balance){
        for(int i=0;i<n;i++){
            accounts.push_back(Account(i,initial_balance[i]));
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
int main()
{
    double a[100];
    for(int i=0;i<100;i++){
        a[i]=1000.0;
    }
    Bank bank(100,a);
    bank.transfer(0,1,100.0);

    std::cout<<bank.getTotalBalance()<<std::endl;
    return 0;
}