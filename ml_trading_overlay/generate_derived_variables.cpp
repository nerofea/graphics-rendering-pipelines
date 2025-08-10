// feature_engineering.cpp
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <cmath>

struct Row { std::string time_iso; double open, high, low, close, volume; };

static std::vector<double> ema(const std::vector<double>& x, int n) {
    std::vector<double> e(x.size(), 0.0);
    if (x.empty() || n<=0) return e;
    double a = 2.0/(n+1.0);
    e[0] = x[0];
    for (size_t i=1;i<x.size();++i) e[i] = a*x[i] + (1-a)*e[i-1];
    return e;
}

static std::vector<double> rsi14(const std::vector<double>& c) {
    const int N=14;
    std::vector<double> r(c.size(), NAN);
    if (c.size()<=N) return r;
    double g=0,l=0;
    for (int i=1;i<=N;++i){ double d=c[i]-c[i-1]; if(d>=0) g+=d; else l-=d; }
    double ag=g/N, al=l/N;
    r[N] = (al==0)?100.0:100.0 - (100.0/(1.0 + ag/al));
    for (size_t i=N+1;i<c.size();++i){
        double d=c[i]-c[i-1];
        double G = d>0?d:0, L = d<0?-d:0;
        ag = (ag*(N-1)+G)/N;
        al = (al*(N-1)+L)/N;
        double rs = (al==0)?INFINITY:ag/al;
        r[i] = 100.0 - (100.0/(1.0+rs));
    }
    return r;
}

static bool parse_csv_row(const std::string& line, Row& out) {
    // Expect: time_iso,open,high,low,close,volume  (header allowed)
    std::stringstream ss(line);
    std::string tok;
    std::vector<std::string> t;
    while (std::getline(ss, tok, ',')) t.push_back(tok);
    if (t.size()<6) return false;
    // skip header
    if (!std::isdigit(t[1].empty()? 'x' : t[1][0]) && t[1]!="0") return false;
    out.time_iso = t[0];
    out.open  = std::stod(t[1]);
    out.high  = std::stod(t[2]);
    out.low   = std::stod(t[3]);
    out.close = std::stod(t[4]);
    out.volume= std::stod(t[5]);
    return true;
}

int main(int argc, char** argv){
    if (argc<3){
        std::cerr << "Usage: " << argv[0] << " <input_raw_ohlcv.csv> <output_features.csv>\n";
        std::cerr << "Expected input columns: time_iso,open,high,low,close,volume\n";
        return 1;
    }
    const std::string inpath=argv[1], outpath=argv[2];

    std::ifstream in(inpath);
    if(!in){ std::cerr<<"Cannot open "<<inpath<<"\n"; return 1; }

    std::string line;
    std::vector<Row> rows; rows.reserve(2000);

    // try to read header
    if (std::getline(in,line)) {
        Row r;
        if (parse_csv_row(line,r)) rows.push_back(r); // if first line was data, keep it
    }
    while (std::getline(in,line)){
        Row r; if(parse_csv_row(line,r)) rows.push_back(r);
    }
    if (rows.size()<20){ std::cerr<<"Not enough rows.\n"; return 1; }

    std::vector<double> closes; closes.reserve(rows.size());
    for (auto& r: rows) closes.push_back(r.close);

    // features
    std::vector<double> ret(rows.size(), NAN);
    for (size_t i=1;i<rows.size();++i) ret[i]=(closes[i]/closes[i-1])-1.0;
    auto ema20 = ema(closes,20);
    auto ema50 = ema(closes,50);
    auto rsi   = rsi14(closes);

    std::ofstream out(outpath);
    if(!out){ std::cerr<<"Cannot open "<<outpath<<"\n"; return 1; }
    out << "time_iso,open,high,low,close,volume,return,ema20,ema50,rsi14\n";
    for (size_t i=0;i<rows.size();++i){
        out << rows[i].time_iso << ","
            << rows[i].open << ","
            << rows[i].high << ","
            << rows[i].low  << ","
            << rows[i].close<< ","
            << rows[i].volume<< ","
            << (std::isnan(ret[i])?0.0:ret[i]) << ","
            << ema20[i] << ","
            << ema50[i] << ","
            << (std::isnan(rsi[i])?0.0:rsi[i]) << "\n";
    }
    std::cout << "Wrote features to " << outpath << "\n";
    return 0;
}
