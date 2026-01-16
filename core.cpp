#include<iostream>
#include<vector>
#include<string>
#include<thread>
#include<mutex>
#include<cstdlib>
#include<shared_mutex>
#include<atomic>

class Account{
private:
    
public:
    int id;
    double balance;
    mutable std::shared_mutex accountmtx;//加锁之后，不可以使用vector<Account>,因为锁不可以被移动和复制
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

        if(from== to) return ;

        //std::lock(accounts[from]->accountmtx,accounts[to]->accountmtx);

        //std::lock_guard<std::mutex> lock1(accounts[from]->accountmtx,std::adopt_lock);
        //std::lock_guard<std::mutex> lock2(accounts[to]->accountmtx,std::adopt_lock);
        //这种写法的意思是同时上锁-自动解锁
        //可以防止死锁
        //在现代cpp中有更智能的写法：
        std::scoped_lock lock(accounts[from]->accountmtx,accounts[to]->accountmtx);

        accounts[from]->balance-=amount;
        accounts[to]->balance+=amount;
        //函数结束自然解锁
    }

    double getTotalBalance(){
        double sum=0;
    {
        for(int i=0;i<accounts.size();i++){
            std::shared_lock<std::shared_mutex> lock(accounts[i]->accountmtx);
            //accounts[i]->accountmtx.lock();
            sum+=accounts[i]->balance;
            //accounts[i]->accountmtx.unlock();
        }
    }
        return sum;
    }
};
void manage(Bank& bank,std::atomic<bool>& isrunning){
    for(int i=0;i<1000;i++){
        int from,to;
        do{
            from=rand()%100;
            to=rand()%100;
        }while(from==to);
        double amount=(double)(rand()%10+1);

        bank.transfer(from,to,amount);
        //当前版本出现了死锁，为什么呢？因为在线程中两个线程可能互相持有对方下一步操作的锁
    }
}
void check(Bank& bank,std::atomic<bool>& isrunning){
    while(isrunning){
        double total=bank.getTotalBalance();

        std::cout<<"total balance:"<<total<<std::endl;

        std::this_thread::sleep_for(std::chrono::milliseconds(20));
    }
}
int main()
{
    srand(time(0));

    std::thread threads[10];
    Bank bank(100,1000.0);
    std::cout<<"initial:"<<bank.getTotalBalance()<<std::endl;

    std::thread auditor;
    std::atomic<bool> isrunning(true);

    for(int i=0;i<10;i++){
        threads[i]=std::thread(manage,std::ref(bank),std::ref(isrunning));
    }
    
    auditor=std::thread(check,std::ref(bank),std::ref(isrunning));

    for(int i=0;i<10;i++){
        if(threads[i].joinable()) threads[i].join();
    }

    isrunning=false;
    if(auditor.joinable()) auditor.join();
    std::cout<<"finally:"<<bank.getTotalBalance()<<std::endl;
    return 0;
    //最后一版代码
    //实现了自动解锁，基本满足了高并发场景下的要求
    //存在一定问题，如一致性快照丢失（inconsistent snapshot）
    //通俗来讲，就是查看线程速度有时追不上处理线程速度
    //可以通过更为底层的CAS操作解决这一问题
}