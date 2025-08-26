# include <iostream>
# include <memory>

void DeletePtr(int* p){
    delete p;
}

int main(){
    //两种声明方式
    auto sp1 = std::make_shared<int>(100);

    std::shared_ptr<int> sp2(new int(100));

    //std::shared_ptr<int> sp = new int(1);
    //不能直接指向一个原始指针
    //但是 int* p = sp.get()  可以获得智能指针管理的 裸指针 地址

    //可以这样
    std::shared_ptr<int> sp3;
    sp3.reset(new int(1));
    //释放资源，不传参数
    //sp3.reset()  sp3.use_count --;

    //可以指定删除器
    std::shared_ptr<int> sp4(new int(1), DeletePtr);
    //管理数组需要指定删除器，因为默认不支持
    std::shared_ptr<int> sp5(new int[10], [](int *p){delete p;});


    //unipue_ptr 独占智能指针
    //不能复制 = 


    //weak_ptr 结合 shared_ptr 使用
    


}