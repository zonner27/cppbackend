#pragma once
#ifdef _WIN32
#include <sdkddkver.h>
#endif

#include <boost/asio/io_context.hpp>
#include <boost/asio/steady_timer.hpp>
#include <boost/asio/strand.hpp>
#include <boost/asio/bind_executor.hpp>
#include <memory>
#include <iostream>
#include <thread>

#include "hotdog.h"
#include "result.h"

namespace net = boost::asio;
using Timer = net::steady_timer;
using namespace std::literals;
using namespace std::chrono;

using Strand = net::strand<net::io_context::executor_type>;

class Coock_Sausage : public std::enable_shared_from_this<Coock_Sausage> {
public:
    explicit Coock_Sausage(net::io_context& io, std::shared_ptr<Sausage> sausage, Strand strand)
        : io_{io},  sausage_(sausage), strand_(strand) {
    }
    using Handler = std::function<void()>;
    void dosausage(Handler handler) {
        sausage_timer_.async_wait(
            net::bind_executor(strand_, [self = shared_from_this(), handler = std::move(handler)](sys::error_code ec) {
            if (ec) {
                throw std::runtime_error("Wait error: "s + ec.message());
            } else
            handler();
        }));
    }

private:
    net::io_context& io_;
    std::shared_ptr<Sausage> sausage_;
    Timer sausage_timer_{io_, 1500ms};
    Strand strand_;
};

class Coock_Bread : public std::enable_shared_from_this<Coock_Bread> {
public:
    explicit Coock_Bread(net::io_context& io, std::shared_ptr<Bread> bread, Strand strand)
        : io_{io},  bread_(bread), strand_(strand){
    }
    using Handler = std::function<void()>;
    void dobread(Handler handler) {
        bread_timer_.async_wait(
            net::bind_executor(strand_, [self = shared_from_this(), handler = std::move(handler)](sys::error_code ec) {
            if (ec) {
                throw std::runtime_error("Wait error: "s + ec.message());
            } else
            handler();
        }));
    }

private:
    net::io_context& io_;
    std::shared_ptr<Bread> bread_;
    Timer bread_timer_{io_, 1000ms};
    Strand strand_;
};

// Функция-обработчик операции приготовления хот-дога
using HotDogHandler = std::function<void(Result<HotDog> hot_dog)>;

// Класс "Кафетерий". Готовит хот-доги
class Cafeteria {
public:
    explicit Cafeteria(net::io_context& io)
        : io_{io} {
    }

    // Асинхронно готовит хот-дог и вызывает handler, как только хот-дог будет готов.
    // Этот метод может быть вызван из произвольного потока
    void OrderHotDog(HotDogHandler handler) {
        // TODO: Реализуйте метод самостоятельно

        std::shared_ptr<Sausage> sausage_;
        std::shared_ptr<Bread> bread_ ;
        {
            std::lock_guard lk{mut};
            sausage_ = store_.GetSausage();
            bread_ = store_.GetBread();
            //std::cout << "sausage = " << sausage_->GetId() << '\n';
            //std::cout << "bread = " << bread_->GetId() << '\n';
        }

        net::dispatch(strand_, [this, sausage_, bread_, handler]() {
            sausage_->StartFry(*gas_cooker_, [this, sausage_, bread_, handler]() {
                const auto start_time = Clock::now();
                //std::cout << "start timer sausaage " << sausage_->GetId() << '\n';
                auto cookS = std::make_shared<Coock_Sausage>(io_, sausage_, strand_);
                cookS->dosausage([cookS, sausage_, start_time, bread_, handler, this] {
                    // Обработчик, который будет выполнен по истечении времени таймера
                    //std::cout << "Sausage is done !" << sausage_->GetId() << std::endl;
                    sausage_->StopFry();
                    const auto cook_duration = Clock::now() - start_time;
                    //std::cout << "stop sausagge timer = " << sausage_->GetId() << "\t  t = " << duration_cast<std::chrono::duration<double>>(cook_duration).count() << "s "
                    //          << "\t " << std::this_thread::get_id() << std::endl;
                    MakeHotDog(sausage_, bread_, handler);
                });
            });
        });

        net::dispatch(strand_, [this, sausage_, bread_, handler]() {
            bread_->StartBake(*gas_cooker_, [this, bread_, handler, sausage_]() {
                const auto start_time = Clock::now();
                //std::cout << "start timer sausaage " << bread_->GetId() << '\n';
                auto cookB = std::make_shared<Coock_Bread>(io_, bread_, strand_);
                cookB->dobread([cookB, bread_, start_time, this, handler, sausage_] {
                    //std::cout << "Bread is done! - " << bread_->GetId() << std::endl;
                    bread_->StopBaking();
                    const auto cook_duration = Clock::now() - start_time;
                    //std::cout << "stop sausagge timer = " << bread_->GetId() << "\t  t = " << duration_cast<std::chrono::duration<double>>(cook_duration).count() << "s "
                    //          << "\t " << std::this_thread::get_id() << std::endl;
                    MakeHotDog(sausage_, bread_, handler);
                });
            });
        });

    }

private:
    net::io_context& io_;
    // Используется для создания ингредиентов хот-дога
    Store store_;
    std::mutex mut;
    // Газовая плита. По условию задачи в кафетерии есть только одна газовая плита на 8 горелок
    // Используйте её для приготовления ингредиентов хот-дога.
    // Плита создаётся с помощью make_shared, так как GasCooker унаследован от
    // enable_shared_from_this.
    std::shared_ptr<GasCooker> gas_cooker_ = std::make_shared<GasCooker>(io_);

    Strand strand_{net::make_strand(io_)};

    std::atomic<int> hotdogId = 0;
    void MakeHotDog (std::shared_ptr<Sausage> sausage, std::shared_ptr<Bread> bread, HotDogHandler handler) {
        if (sausage->IsCooked() && bread->IsCooked()) {
             //std::cout << "CAN do HOTDOG!" << std::endl;
             //std::cout << "sausage->GetId() = " << sausage->GetId() << std::endl;
             //std::cout << "bread_->GetId() = " << bread->GetId() << std::endl;
             std::shared_ptr<HotDog> hotdog_;
             hotdog_ = std::make_shared<HotDog>(++hotdogId, sausage, bread);;
             //std::cout << "Result HotDog ID: " << hotdog_->GetId() << std::endl;
             handler(Result<HotDog>(std::move(*hotdog_)));
        } else {
            //std::cout << "not doit\n";
        }
    }

};
