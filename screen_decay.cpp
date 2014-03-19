#include <iostream>
#include <fstream>
#include <string>
#include <cstdlib>
#include <vector>
#include <cmath>

#include "mtwist.h"

MTwist rng;

using namespace std;

struct Entity {
    int n_tweets;
    int ID;
    double tweet_rate;
    vector<double> time_of_tweets, time_kicked_out, time_on_screen;
    
    Entity() {
        n_tweets = ID = 0;
        tweet_rate = 0;
    }
};

typedef std::vector<Entity> Entities;
Entities entities;

struct MainInfo {
    double final_time;
    double time;
    int n_following, n_tweets, n_steps, n_types;
    vector<double> tweet_rates, norm_tweet_rates;
    vector<int> tweet_type_size, range_min, range_max;
    
    double progress_time() {
        double rand_num = rng.rand_real_not0();
        double total_rate = 0;
        for (int i = 0; i < n_types; i ++) {
            total_rate += tweet_rates[i]*tweet_type_size[i];
        }
        return (-log(rand_num) / (total_rate));
    }
    
    MainInfo () {
        // 2 rates
        final_time = time = 0;
        n_following = n_tweets = n_steps = n_types = 0;
    }
};

MainInfo main_info;

struct Screen {
    vector<int> entities_on_screen;
    double average;
    Screen () {
        average = -1;
    }
    
    
    
    void kick_out_entity() {
        // if size gets bigger than 20, remove oldest entity
        if (entities_on_screen.size() > 20) {
            int ID_removed = entities_on_screen[0];
            entities[ID_removed].time_kicked_out.push_back(main_info.time);
            entities_on_screen.erase(entities_on_screen.begin());
            double time = entities[ID_removed].time_kicked_out.back() - entities[ID_removed].time_of_tweets[entities[ID_removed].time_kicked_out.size() - 1];
            entities[ID_removed].time_on_screen.push_back(time);
            
        }
    }
};

Screen screen;

void normalize_tweet_rates() {
    double sum = 0;
    for (int i = 0; i < main_info.tweet_rates.size(); i ++) {
        sum += main_info.tweet_rates[i];
    }
    for (int i = 0; i < main_info.tweet_rates.size(); i ++) {
        main_info.norm_tweet_rates[i] = main_info.tweet_rates[i] / sum;
    }
}

void fix_ID_range() {
    main_info.range_min[0] = 0;
    main_info.range_max[0] = main_info.tweet_type_size[0];
    for (int i = 1; i < main_info.n_types; i ++) {
        main_info.range_min[i] = main_info.range_max[i-1];
        main_info.range_max[i] = main_info.range_min[i] + main_info.tweet_type_size[i];
    }
}
    
void declare_entities() {
    for (int i = 0; i < entities.size(); i ++) {
        Entity& e = entities[i];
        e.ID = i;
        for (int j = 0; j < main_info.range_max.size(); j ++) {
            if (e.ID > main_info.range_min[j] && e.ID < main_info.range_max[j]) {
                e.tweet_rate = main_info.tweet_rates[j];
            }
        }
    }
}

void prompt_for_info() {
    cout << "\nWelcome to the tweet 'screen-space' simulator.\n\n";
    cout << "Please provide some further information.\n\n";
    cout << "Number of different tweet types: ";
    cin >> main_info.n_types;
    main_info.tweet_rates.resize(main_info.n_types);
    main_info.norm_tweet_rates.resize(main_info.n_types);
    main_info.tweet_type_size.resize(main_info.n_types);
    main_info.range_min.resize(main_info.n_types);
    main_info.range_max.resize(main_info.n_types);
    for (int i = 0; i < main_info.n_types; i ++) {
        printf("Rate of tweeting for tweet type %i: ", i);
        cin >> main_info.tweet_rates[i];
        printf("Number of entities for tweet type %i: ", i);
        cin >> main_info.tweet_type_size[i];
        main_info.n_following += main_info.tweet_type_size[i];
    }
    entities.resize(main_info.n_following);
    declare_entities();
    cout << "Final time for the simulation (min): ";
    cin >> main_info.final_time;
    cout << "\nThank you.\n\n";
    normalize_tweet_rates();
    fix_ID_range();
}

void handle_tweet(int ID) {
    screen.entities_on_screen.push_back(ID);
    entities[ID].time_of_tweets.push_back(main_info.time);
    entities[ID].n_tweets ++;
    main_info.n_tweets ++;
    screen.kick_out_entity();
}

void display_screen(ofstream& output) {
    output<< "\n\nTime: " << main_info.time << "\n\n";
    output<< " _______________________________________\n _______________________________________\n";
    for (int i = 0; i < screen.entities_on_screen.size(); i ++) {
        output<< "|\t\t\t\t\t|\n";
        output<< "| Tweet #: " << i << " ID: " << screen.entities_on_screen[i] << "\tN_tweets: " << entities[screen.entities_on_screen[i]].n_tweets << "\t|\n";
        output<< "|_______________________________________|\n";
    }
}

void run_simulation() {
    double rand_num = rng.rand_real_not0();
    for (int i = 0; i < main_info.norm_tweet_rates.size(); i ++) {
        if (rand_num <= main_info.norm_tweet_rates[i]) {
            int entity_tweeting = rng.rand_int(main_info.range_min[i], main_info.range_max[i]);
            handle_tweet(entity_tweeting);
            break;
        }
        rand_num -= main_info.norm_tweet_rates[i];
    }
    main_info.time += main_info.progress_time();    
}

void print_info() {
    vector<double> avg_times;
    cout << "\n\nSimulation Info:\n__________________________\n\n";
    printf("Following: %d\n", main_info.n_following);
    printf("Average number of tweets per entity: %f\n", main_info.n_tweets / (double) main_info.n_following);
    for (int i = 0; i < main_info.n_following; i ++) {
        double time_sum = 0;
        Entity& e = entities[i];
            for (int j = 0; j < e.time_on_screen.size(); j ++) {
                time_sum += e.time_on_screen[j];
            }
            printf("Entity %i lasted %f minutes on average.\n", i, time_sum/(double)e.time_on_screen.size());
            avg_times.push_back(time_sum/(double)e.time_on_screen.size());
    }
    int max = 0;
    for (int i = 0; i < avg_times.size(); i ++) {
        if (avg_times[i] > max) {
            max = avg_times[i];
        }
    }
    vector<int> bins(max + 1);
    for (int i = 0; i < bins.size(); i ++) {
        bins[i] = 0;
    }
    for (int i = 0; i < avg_times.size(); i ++) {
        int val = round(avg_times[i]);
        bins[val] ++;
    }
    ofstream out;
    out.open("distro.dat");
    for (int i = 0; i < bins.size(); i ++) {
        out << i << "\t" << bins[i] << "\n";
    }
}

bool main_loop() {
    if (main_info.time < main_info.final_time) {
        return true;
    }
    return false;
}

int main() {
    ofstream output;
    output.open("screen.dat");
    prompt_for_info();
    while (main_loop()) {
        run_simulation();
        display_screen(output);
    }
    print_info();
    output.close();
    return 0;
}