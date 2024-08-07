
// TODO
//
// implementing stealmate by repeating turns
//
// we could further reduce the board representations

///
//////
/////////// settings
//////
///

#define BOT_BRUTE_FORCE_DEPTH 3
// how many turns into the future is the bot going to attempt to look
// 0 means only consider what you can take on the current turn

#define BOT_MAX_NUMBER_OF_MATERIAL_EXTENSIONS 2
// how many more times can we extend the number of turns we can look into the future
// if we have more material

#define USE_SAVE_FILE false
#define SAVE_FILE "saved-moves.sex"
#define SAVE_CHANCE (RAND_MAX / 10 * 1)

#define REMEMBER_MOVES (USE_SAVE_FILE && false)

///
//////
/////////// include
//////
///

#include <iostream>
#include <vector>
#include <cassert>
#include <random>
#include <climits>
// #include <map>
#include <unordered_map>
#include <fstream>
#include <filesystem>
#include <mutex>
#include <shared_mutex>
#include <thread>
#include <algorithm>

///
//////
/////////// namespace
//////
///

using namespace std;

///
//////
/////////// macro: generic
//////
///

#define DBG(...) { \
    cout << "DBG: "; \
    cout << __VA_ARGS__; \
    cout << endl; \
}

#define ERR(...) { \
    cout << "ERROR:" << endl; \
    cout << __VA_ARGS__ << endl; \
    cout << "(" << endl; \
    cout << "    location:" << endl; \
    cout << "        file `" << __FILE__ << "`" << endl; \
    cout << "        line " << __LINE__ << endl; \
    cout << "        function `" << __FUNCTION__ << "`" << endl; \
    cout << "        commit id `" << COMMIT_ID << "`" << endl; \
    cout << ")" << endl; \
    cout << endl; \
    exit(1); \
}

#define UNREACHABLE() { \
    ERR("unreachable"); \
}

///
//////
/////////// define: icon
//////
///

// #define ICON_PAWN_WHITE   "♙"
// #define ICON_KNIGHT_WHITE "♘"
// #define ICON_BISHOP_WHITE "♗"
// #define ICON_ROOK_WHITE   "♖"
// #define ICON_QUEEN_WHITE  "♕"
// #define ICON_KING_WHITE   "♔"

// #define ICON_PAWN_BLACK   "♟︎"
// #define ICON_KNIGHT_BLACK "♞"
// #define ICON_BISHOP_BLACK "♝"
// #define ICON_ROOK_BLACK   "♜"
// #define ICON_QUEEN_BLACK  "♛"
// #define ICON_KING_BLACK   "♚"

// #define ICON_WARNING "⚠"

#define ICON_PAWN   "♟︎"
#define ICON_KNIGHT "♞"
#define ICON_BISHOP "♝"
#define ICON_ROOK   "♜"
#define ICON_QUEEN  "♛"
#define ICON_KING   "♚"

///
//////
/////////// enum + typedef
//////
///

enum piece_type{
    PT_PAWN,
    PT_KNIGHT,
    PT_BISHOP,
    PT_ROOK,
    PT_QUEEN,
    PT_KING,
};

enum winner{
    WINNER_NO_WINNER_YET,
    WINNER_PLAYER_0,
    WINNER_PLAYER_1,
    WINNER_STALEMATE,
};

typedef pair<int, int> board_pos_t;

typedef pair<board_pos_t, board_pos_t> board_move_simple_t;

typedef vector<board_move_simple_t> board_move_t;

///
//////
/////////// declaration
//////
///

class Tile;

class Board{

    public:

        vector<Tile *> tiles = {};

        int player_turn = 0; // which player's turn is it

        unordered_map< string , vector< board_move_simple_t > > * already_calculated_moves = nullptr;
        shared_mutex * already_calculated_moves_lock = nullptr;

        // Board();

        ~Board();

        void init();

        Board * duplicate();

        void draw();

        enum winner next_turn(bool original_call, int brute_force_depth, int material_extensions_left);

        int count_material(int for_player);

        pair<bool, Tile *> get_tile_at(board_pos_t pos);

        enum winner make_move(board_move_t move);

        string get_state(int arg_player_turn, int additional_depth, int extensions_performed);

        void save_calculated_moves(string file);

        void load_calculated_moves(string file);

        board_pos_t terminal_position_to_board_position(pair<int, int> term_pos);

    private:

        pair<bool, ssize_t> calc_idx(int y, int x);

        void set_tile_at(int y, int x, Tile * tile);

        void spawn_tiles();

        void connect_neighbours();

        void place_pieces();
};

class Piece{

    public:

        int forward_y;
        // int forward_x = 0; // this only makes sense in 4way chess

        int owner; // which player owns the piece

        enum piece_type type;

        Tile * location;

        bool has_not_moved = true;

        Piece * duplicate(Tile * arg_location);

        vector<board_move_t> get_valid_moves();

        enum winner move_to(Board * board, board_pos_t pos);

        board_pos_t get_pos();

        void draw();

        unsigned char get_representation(); // includes the type of the piece and the owner

    private:

        vector<board_move_t> gen_valid_moves_pawn();

        vector<board_move_t> gen_valid_moves_knight();

        vector<board_move_t> gen_valid_moves_bishop();

        vector<board_move_t> gen_valid_moves_rook();

        vector<board_move_t> gen_valid_moves_king();

};

class Tile{

    public:

        int y;
        int x;

        // horizontal/vertical
        Tile * neighbour_up = nullptr;
        Tile * neighbour_down = nullptr;
        Tile * neighbour_left = nullptr;
        Tile * neighbour_right = nullptr;
        // diagonal
        Tile * neighbour_upleft = nullptr;
        Tile * neighbour_upright = nullptr;
        Tile * neighbour_downleft = nullptr;
        Tile * neighbour_downright = nullptr;

        Piece * piece = nullptr;

        Tile * duplicate();

        board_pos_t get_pos();

};

///
//////
/////////// function: generic
//////
///

int random_int(int from, int to){
    random_device rd;
    mt19937 gen(rd()); // seed
    uniform_int_distribution<> distr(from, to);
    return distr(gen);
}

///
//////
/////////// template: generic
//////
///

template<typename T>
T vec_get_random_element(vector<T>& vec){
    int length = static_cast<int>(vec.size());
    size_t index = random_int(0, length - 1);
    return vec[index];
}

///
//////
/////////// function: terminal
//////
///

void term(char * command) {
    int ret_code = system(command);
    if(ret_code != 0){
        ERR("command failed: `" << command << "`");
    }
}

// mouse click

void terminal_mouse_click_log_enable() {
    printf("\033[?9h"); // the line on the bottom doesnt work without this one
    printf("\033[?1006h");
}

void terminal_mouse_click_log_disable() {
    printf("\033[?1006l");
    printf("\033[?9l");
}

// echo

void terminal_echo_enable() {
    term((char*)"stty echo");
}

void terminal_echo_disable() {
    term((char*)"stty -echo");
}

// line buffering

void terminal_line_buffering_enable() {
    term((char*)"stty icanon");
}

void terminal_line_buffering_disable() {
    term((char*)"stty -icanon");
}

///
//////
/////////// function: input
//////
///

string input_string(){
    string data;
    getline(cin, data);
    return data;
}

void input_enter(){
    input_string();
}

int input_int(){
    string data = input_string();
    return stoi(data); // this CAN crash
}

board_pos_t input_chess_pos(string prompt){

    while(true){

        cout << prompt;

        string data = input_string();

        if(data.size() != 2){
            cout << "try again: length needs to be 2" << endl;
            continue;
        }

        char x_char = data.at(0);
        char y_char = data.at(1);

        if(x_char < 'a' || x_char > 'h'){
            cout << "try again: x coordinate needs to be between `a` and `h`" << endl;
            continue;
        }

        int x = x_char - 'a';

        if(y_char < '1' || y_char > '8'){
            cout << "try again: y coordinate needs to be between `1` and `8`" << endl;
            continue;
        }

        int y = y_char - '1';
        y = 7 - y;

        return {y, x};

    }

}

///
//////
/////////// function: input mouse
//////
///

// some info on the input modes: https://invisible-island.net/xterm/ctlseqs/ctlseqs.html#h3-Extended-coordinates

#define CSI {'\033', '['}
#define CSI_LEN 2

#define SGR_BEGIN '<'
#define SGR_SEP ';'
#define SGR_END 'M'

pair<int, int> input_mouse_click() {

    terminal_line_buffering_disable();
    terminal_echo_disable();
    terminal_mouse_click_log_enable();

    int mouse_y = 0;
    int mouse_x = 0;

    for(;;){

        string line;
        // printf("\n");
        // cout << "byte#0: " << (int)line[0] << '\n';
        // cout << "byte#1: " << (int)line[1] << '\n';
        // cout << "byte#2: " << (int)line[2] << '\n';
        // cout << "byte#3: " << (int)line[3] << '\n';
        // cout << "byte#4: " << (int)line[4] << '\n';
        // cout << "byte#5: " << (int)line[5] << '\n';
        // cout << "byte#6: " << (int)line[6] << '\n';
        // cout << "byte#7: " << (int)line[7] << '\n';

        for(;;){
            char ch = static_cast<char>(getchar());
            
            line += ch;

            if(ch == SGR_END){

                ssize_t csi_idx = line.rfind(CSI);

                if(csi_idx < 0){
                    continue;
                }

                line.erase(0, csi_idx + CSI_LEN);

                break;
            }
        }

        assert(line[0] == SGR_BEGIN);
        line.erase(0, 1);

        // this really should not be an assert since this can be 0=click 64=mwheelup 65=mwheeldown
        assert(line[0] == '0');
        line.erase(0, 1);
        assert(line[0] == SGR_SEP);
        line.erase(0, 1);

        mouse_y = 0;
        mouse_x = 0;

        // parse mouse_x
        for(;;){
            char ch = line[0];
            line.erase(0, 1);

            if(ch == SGR_SEP){
                break;
            }

            assert(ch >= '0');
            assert(ch <= '9');

            mouse_x *= 10;
            mouse_x += ch - '0';
        }

        // parse mouse_y
        for(;;){
            char ch = line[0];
            line.erase(0, 1);

            if(ch == SGR_END){
                break;
            }

            assert(ch >= '0');
            assert(ch <= '9');

            mouse_y *= 10;
            mouse_y += ch - '0';
        }

        // ...

        assert(line.length() == 0);

        break;

    }

    terminal_mouse_click_log_disable();
    terminal_echo_enable();
    terminal_line_buffering_enable();

    // correct the positions, from 1-indexed, to 0-indexed
    mouse_y -= 1;
    mouse_x -= 1;

    return {mouse_y, mouse_x};
}

///
//////
/////////// function: file IO
//////
///

// TODO almost all of these are endianness-specific

// write

void file_write_size(ofstream & f, size_t data){
    f.write(reinterpret_cast<char *>(&data), sizeof(data));
    assert(!f.fail());
}

void file_write_string(ofstream & f, const string & data){
    size_t size = data.size();
    file_write_size(f, size);

    // DBG("writing string of size " << size);
    // input_enter();

    f.write(data.c_str(), size);
    assert(!f.fail());
}

void file_write_boardpos(ofstream & f, board_pos_t & data){
    f.write(reinterpret_cast<char *>(&data.first), sizeof(int));
    assert(!f.fail());
    f.write(reinterpret_cast<char *>(&data.second), sizeof(int));
    assert(!f.fail());
}

void file_write_vector_boardmovesimple(ofstream & f, vector< board_move_simple_t > & data){
    file_write_size(f, data.size());

    for(auto [pos0, pos1] : data){
        file_write_boardpos(f, pos0);
        assert(!f.fail());

        file_write_boardpos(f, pos1);
        assert(!f.fail());
    }
}

void file_write_unsortedmap_string_vector_boardmovesimple(ofstream & f, unordered_map< string , vector< board_move_simple_t > > * data){

    file_write_size(f, data->size());

    for(auto it = data->begin(); it != data->end(); ++it){
        file_write_string(f, it->first);
        file_write_vector_boardmovesimple(f, it->second);
    }
}

// read

void file_read_size(ifstream & f, size_t & data){
    f.read(reinterpret_cast<char *>(&data), sizeof(data));
    assert(f.gcount() == sizeof(data));
}

void file_read_string(ifstream & f, string & data){
    size_t size = 0;
    file_read_size(f, size);

    char * data_cstr = new char[size+1];

    f.read(data_cstr, size);
    assert(f.gcount() == static_cast<ssize_t>(size));

    data_cstr[size] = 0;

    data = data_cstr;
}

void file_read_boardpos(ifstream & f, board_pos_t & data){
    f.read(reinterpret_cast<char *>(&data.first), sizeof(int));
    assert(f.gcount() == sizeof(int));

    f.read(reinterpret_cast<char *>(&data.second), sizeof(int));
    assert(f.gcount() == sizeof(int));
}

void file_read_vector_boardmovesimple(ifstream & f, vector< board_move_simple_t > & data){
    size_t size = 0;
    file_read_size(f, size);

    for(size_t i=0; i<size; ++i){
        board_pos_t pair0 = {};
        board_pos_t pair1 = {};

        file_read_boardpos(f, pair0);
        file_read_boardpos(f, pair1);

        data.push_back({pair0, pair1});
    }
}

void file_read_unsortedmap_string_vector_boardmovesimple(ifstream & f, unordered_map< string , vector< board_move_simple_t > > * data){
    size_t size = 0;
    file_read_size(f, size);

    for(size_t i=0; i<size; ++i){
        string key = {};
        file_read_string(f, key);

        vector<board_move_simple_t> value = {};
        file_read_vector_boardmovesimple(f, value);

        (*data)[key] = value;
    }
}

///
//////
/////////// module: terminal escape code
//////
///

#define COL_RESET "\033[0m"

#define COL_INT(color_number) ("\033[38;5;" #color_number "m")

#define COL_PLAYER_0 COL_INT(40)
#define COL_PLAYER_1 COL_INT(196)

void disp_clear(){
    cout << "\033[H\033[J";
}

void disp_cur_set(int y, int x){
    cout << "\033[";
    cout << y + 1;
    cout << ";";
    cout << x + 1;
    cout << "H";
}

///
//////
/////////// module: piece
//////
///

Piece * Piece::duplicate(Tile * arg_location){

    Piece * copy = new Piece;

    copy->forward_y = forward_y;

    copy->owner = owner;

    copy->type = type;

    copy->location = arg_location;

    copy->has_not_moved = has_not_moved;

    return copy;

}

vector<board_move_t> Piece::get_valid_moves(){

    vector<board_move_t> moves = {};

    switch(type){

        case PT_PAWN:{
            moves = gen_valid_moves_pawn();
        }break;

        case PT_KNIGHT:{
            moves = gen_valid_moves_knight();
        }break;

        case PT_BISHOP:{
            moves = gen_valid_moves_bishop();
        }break;

        case PT_ROOK:{
            moves = gen_valid_moves_rook();
        }break;

        case PT_QUEEN:{
            moves = gen_valid_moves_bishop();
            for(board_move_t rook_move : gen_valid_moves_rook()){
                moves.push_back(rook_move);
            }
        }break;

        case PT_KING:{
            moves = gen_valid_moves_king();
        }break;

    }

    return moves;

}

enum winner Piece::move_to(Board * board, board_pos_t pos){

    auto [tile_fail, tile] = board->get_tile_at(pos);
    assert(!tile_fail);

    has_not_moved = false;

    location->piece = nullptr;
    location = tile;

    Piece * piece = tile->piece;
    tile->piece = this;

    if(piece){

        enum piece_type piece_type = piece->type;
        int piece_owner = piece->owner;

        piece->location = nullptr;
        delete piece;

        if(piece_type == PT_KING){
            if(piece_owner == 0){
                return WINNER_PLAYER_1;
            }else if(piece_owner == 1){
                return WINNER_PLAYER_0;
            }else{
                UNREACHABLE();
            }
        }

    }

    // TODO what if this is a pawn and it reached the last tile

    return WINNER_NO_WINNER_YET;

}

board_pos_t Piece::get_pos(){

    return location->get_pos();

}

void Piece::draw(){

    string icon;

    switch(type){
        case PT_PAWN:
            icon = ICON_PAWN;
            break;
        case PT_KNIGHT:
            icon = ICON_KNIGHT;
            break;
        case PT_BISHOP:
            icon = ICON_BISHOP;
            break;
        case PT_ROOK:
            icon = ICON_ROOK;
            break;
        case PT_QUEEN:
            icon = ICON_QUEEN;
            break;
        case PT_KING:
            icon = ICON_KING;
            break;
    }

    string color;

    if(owner == 0){
        color = COL_PLAYER_0;
    }else if(owner == 1){
        color = COL_PLAYER_1;
    }else{
        UNREACHABLE();
    }

    cout << color << icon << COL_RESET;

}

unsigned char Piece::get_representation(){

    unsigned char repr = 0;

    switch(type){
        case PT_PAWN:
            repr = 0x1;
            break;
        case PT_KNIGHT:
            repr = 0x2;
            break;
        case PT_BISHOP:
            repr = 0x3;
            break;
        case PT_ROOK:
            repr = 0x4;
            break;
        case PT_QUEEN:
            repr = 0x5;
            break;
        case PT_KING:
            repr = 0x6;
            break;
    }

    if(owner == 0){
        // do nothing
    }else if(owner == 1){
        repr += 6;
    }else{
        UNREACHABLE();
    }

    return repr;

}

// private

vector<board_move_t> Piece::gen_valid_moves_pawn(){

    vector<board_move_t> moves = {};

    if(forward_y == -1){

        if(location->neighbour_up && !location->neighbour_up->piece){

            moves.push_back({{get_pos(), location->neighbour_up->get_pos()}});

            if(has_not_moved && location->neighbour_up->neighbour_up && !location->neighbour_up->neighbour_up->piece){

                moves.push_back({{get_pos(), location->neighbour_up->neighbour_up->get_pos()}});

            }

        }

        if(location->neighbour_upleft && (location->neighbour_upleft->piece && location->neighbour_upleft->piece->owner != owner)){
            moves.push_back({{get_pos(), location->neighbour_upleft->get_pos()}});
        }

        if(location->neighbour_upright && (location->neighbour_upright->piece && location->neighbour_upright->piece->owner != owner)){
            moves.push_back({{get_pos(), location->neighbour_upright->get_pos()}});
        }

        // TODO en passant

    }else if(forward_y == 1){

        if(location->neighbour_down && !location->neighbour_down->piece){

            moves.push_back({{get_pos(), location->neighbour_down->get_pos()}});

            if(has_not_moved && location->neighbour_down->neighbour_down && !location->neighbour_down->neighbour_down->piece){

                moves.push_back({{get_pos(), location->neighbour_down->neighbour_down->get_pos()}});

            }

        }

        if(location->neighbour_downleft && (location->neighbour_downleft->piece && location->neighbour_downleft->piece->owner != owner)){
            moves.push_back({{get_pos(), location->neighbour_downleft->get_pos()}});
        }

        if(location->neighbour_downright && (location->neighbour_downright->piece && location->neighbour_downright->piece->owner != owner)){
            moves.push_back({{get_pos(), location->neighbour_downright->get_pos()}});
        }

        // TODO en passant

    }else{

        UNREACHABLE();

    }

    return moves;

}

vector<board_move_t> Piece::gen_valid_moves_knight(){

    vector<Tile *> move_candidates = {};

    {

        Tile * loc;

        if((loc = location->neighbour_up)){
            move_candidates.push_back(loc->neighbour_upleft);
            move_candidates.push_back(loc->neighbour_upright);
        }

        if((loc = location->neighbour_down)){
            move_candidates.push_back(loc->neighbour_downleft);
            move_candidates.push_back(loc->neighbour_downright);
        }

        if((loc = location->neighbour_left)){
            move_candidates.push_back(loc->neighbour_upleft);
            move_candidates.push_back(loc->neighbour_downleft);
        }

        if((loc = location->neighbour_right)){
            move_candidates.push_back(loc->neighbour_upright);
            move_candidates.push_back(loc->neighbour_downright);
        }

        if((loc = location->neighbour_upleft)){
            move_candidates.push_back(loc->neighbour_up);
            move_candidates.push_back(loc->neighbour_right);
        }

        if((loc = location->neighbour_upright)){
            move_candidates.push_back(loc->neighbour_up);
            move_candidates.push_back(loc->neighbour_right);
        }

        if((loc = location->neighbour_downleft)){
            move_candidates.push_back(loc->neighbour_down);
            move_candidates.push_back(loc->neighbour_left);
        }

        if((loc = location->neighbour_downright)){
            move_candidates.push_back(loc->neighbour_down);
            move_candidates.push_back(loc->neighbour_right);
        }

    }

    // remove duplicates
    move_candidates.erase( unique( move_candidates.begin(), move_candidates.end() ), move_candidates.end() );

    vector<board_move_t> valid_moves = {};

    for(Tile * tile : move_candidates){

        if(!tile){
            continue;
        }

        if(!tile->piece || tile->piece->owner != owner){
            valid_moves.push_back({{get_pos(), tile->get_pos()}});
        }

    }

    return valid_moves;

}

vector<board_move_t> Piece::gen_valid_moves_bishop(){

    vector<board_move_t> moves = {};

    {
        Tile * pos = location;

        while(true){
            pos = pos->neighbour_upleft;
            if(!pos){
                break;
            }

            if(pos->piece){
                if(pos->piece->owner != owner){
                    moves.push_back({{get_pos(), pos->get_pos()}});
                }

                break;
            }else{
                moves.push_back({{get_pos(), pos->get_pos()}});
            }
        }
    }

    {
        Tile * pos = location;

        while(true){
            pos = pos->neighbour_upright;
            if(!pos){
                break;
            }

            if(pos->piece){
                if(pos->piece->owner != owner){
                    moves.push_back({{get_pos(), pos->get_pos()}});
                }

                break;
            }else{
                moves.push_back({{get_pos(), pos->get_pos()}});
            }
        }
    }

    {
        Tile * pos = location;

        while(true){
            pos = pos->neighbour_downleft;
            if(!pos){
                break;
            }

            if(pos->piece){
                if(pos->piece->owner != owner){
                    moves.push_back({{get_pos(), pos->get_pos()}});
                }

                break;
            }else{
                moves.push_back({{get_pos(), pos->get_pos()}});
            }
        }
    }

    {
        Tile * pos = location;

        while(true){
            pos = pos->neighbour_downright;
            if(!pos){
                break;
            }

            if(pos->piece){
                if(pos->piece->owner != owner){
                    moves.push_back({{get_pos(), pos->get_pos()}});
                }

                break;
            }else{
                moves.push_back({{get_pos(), pos->get_pos()}});
            }
        }
    }

    return moves;

}

vector<board_move_t> Piece::gen_valid_moves_rook(){

    vector<board_move_t> moves = {};

    {
        Tile * pos = location;

        while(true){
            pos = pos->neighbour_up;
            if(!pos){
                break;
            }

            if(pos->piece){
                if(pos->piece->owner != owner){
                    moves.push_back({{get_pos(), pos->get_pos()}});
                }

                break;
            }else{
                moves.push_back({{get_pos(), pos->get_pos()}});
            }
        }
    }

    {
        Tile * pos = location;

        while(true){
            pos = pos->neighbour_down;
            if(!pos){
                break;
            }

            if(pos->piece){
                if(pos->piece->owner != owner){
                    moves.push_back({{get_pos(), pos->get_pos()}});
                }

                break;
            }else{
                moves.push_back({{get_pos(), pos->get_pos()}});
            }
        }
    }

    {
        Tile * pos = location;

        while(true){
            pos = pos->neighbour_left;
            if(!pos){
                break;
            }

            if(pos->piece){
                if(pos->piece->owner != owner){
                    moves.push_back({{get_pos(), pos->get_pos()}});
                }

                break;
            }else{
                moves.push_back({{get_pos(), pos->get_pos()}});
            }
        }
    }

    {
        Tile * pos = location;

        while(true){
            pos = pos->neighbour_right;
            if(!pos){
                break;
            }

            if(pos->piece){
                if(pos->piece->owner != owner){
                    moves.push_back({{get_pos(), pos->get_pos()}});
                }

                break;
            }else{
                moves.push_back({{get_pos(), pos->get_pos()}});
            }
        }
    }

    return moves;

}

vector<board_move_t> Piece::gen_valid_moves_king(){

    vector<board_move_t> moves = {};

    if(location->neighbour_up && (!location->neighbour_up->piece || location->neighbour_up->piece->owner != owner)){
        moves.push_back({{get_pos(), location->neighbour_up->get_pos()}});
    }

    if(location->neighbour_upright && (!location->neighbour_upright->piece || location->neighbour_upright->piece->owner != owner)){
        moves.push_back({{get_pos(), location->neighbour_upright->get_pos()}});
    }

    if(location->neighbour_right && (!location->neighbour_right->piece || location->neighbour_right->piece->owner != owner)){
        moves.push_back({{get_pos(), location->neighbour_right->get_pos()}});
    }

    if(location->neighbour_downright && (!location->neighbour_downright->piece || location->neighbour_downright->piece->owner != owner)){
        moves.push_back({{get_pos(), location->neighbour_downright->get_pos()}});
    }

    if(location->neighbour_down && (!location->neighbour_down->piece || location->neighbour_down->piece->owner != owner)){
        moves.push_back({{get_pos(), location->neighbour_down->get_pos()}});
    }

    if(location->neighbour_downleft && (!location->neighbour_downleft->piece || location->neighbour_downleft->piece->owner != owner)){
        moves.push_back({{get_pos(), location->neighbour_downleft->get_pos()}});
    }

    if(location->neighbour_left && (!location->neighbour_left->piece || location->neighbour_left->piece->owner != owner)){
        moves.push_back({{get_pos(), location->neighbour_left->get_pos()}});
    }

    if(location->neighbour_upleft && (!location->neighbour_upleft->piece || location->neighbour_upleft->piece->owner != owner)){
        moves.push_back({{get_pos(), location->neighbour_upleft->get_pos()}});
    }

    if(has_not_moved){

        // TODO I think you're not allowed to do this if you're under check

        if(location->neighbour_left && !location->neighbour_left->piece){
            if(location->neighbour_left->neighbour_left && !location->neighbour_left->neighbour_left->piece){
                if(location->neighbour_left->neighbour_left->neighbour_left && !location->neighbour_left->neighbour_left->neighbour_left->piece){
                    if(location->neighbour_left->neighbour_left->neighbour_left->neighbour_left){
                        if(location->neighbour_left->neighbour_left->neighbour_left->neighbour_left->piece){
                            if(location->neighbour_left->neighbour_left->neighbour_left->neighbour_left->piece->type == PT_ROOK){
                                if(location->neighbour_left->neighbour_left->neighbour_left->neighbour_left->piece->owner == owner){
                                    if(location->neighbour_left->neighbour_left->neighbour_left->neighbour_left->piece->has_not_moved){
                                        moves.push_back({
                                            {get_pos(), location->neighbour_left->neighbour_left->get_pos()},
                                            {location->neighbour_left->neighbour_left->neighbour_left->neighbour_left->get_pos(), location->neighbour_left->get_pos()},
                                        });
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }

        if(location->neighbour_right && !location->neighbour_right->piece){
            if(location->neighbour_right->neighbour_right && !location->neighbour_right->neighbour_right->piece){
                if(location->neighbour_right->neighbour_right->neighbour_right){
                    if(location->neighbour_right->neighbour_right->neighbour_right->piece){
                        if(location->neighbour_right->neighbour_right->neighbour_right->piece->type == PT_ROOK){
                            if(location->neighbour_right->neighbour_right->neighbour_right->piece->owner == owner){
                                if(location->neighbour_right->neighbour_right->neighbour_right->piece->has_not_moved){
                                    moves.push_back({
                                        {get_pos(), location->neighbour_right->neighbour_right->get_pos()},
                                        {location->neighbour_right->neighbour_right->neighbour_right->get_pos(), location->neighbour_right->get_pos()},
                                    });
                                }
                            }
                        }
                    }
                }
            }
        }

    }



    return moves;

}

///
//////
/////////// module: tile
//////
///

Tile * Tile::duplicate(){

    Tile * copy = new Tile;

    copy->y = y;
    copy->x = x;

    // the neighbours should be initialised by the caller

    if(piece){
        copy->piece = piece->duplicate(copy);
    }

    return copy;

}

board_pos_t Tile::get_pos(){

    return {y, x};

}

///
//////
/////////// module: board
//////
///

// Board::Board(){
// }

Board::~Board(){

    for(Tile * tile : tiles){

        if(tile->piece){
            delete tile->piece;
            tile->piece = nullptr;
        }

        delete tile;

    }

}

void Board::init(){

    already_calculated_moves = new unordered_map< string , vector< board_move_simple_t > > ();
    already_calculated_moves_lock = new shared_mutex ();

    spawn_tiles();
    connect_neighbours();
    place_pieces();

}

Board * Board::duplicate(){

    Board * copy = new Board;

    for(Tile * tile : tiles){
        copy->tiles.push_back(tile->duplicate());
    }

    copy->connect_neighbours();

    copy->player_turn = player_turn;

    shared_lock lock(*already_calculated_moves_lock); // lock for reading

    copy->already_calculated_moves = already_calculated_moves;

    copy->already_calculated_moves_lock = already_calculated_moves_lock;

    lock.unlock();

    return copy;

}

void Board::draw(){

    disp_clear();

    for(Tile * tile : tiles){

        int draw_y = tile->y * 2 + 1;
        int draw_x = tile->x * 4 + 2;

        { // draw borders (ineffeciently)

            disp_cur_set(draw_y, draw_x - 2);
            cout << "|";

            disp_cur_set(draw_y, draw_x + 2);
            cout << "|";

            disp_cur_set(draw_y - 1, draw_x - 1);
            cout << "---";

            disp_cur_set(draw_y + 1, draw_x - 1);
            cout << "---";

        }

        Piece * piece = tile->piece;
        if(!piece){
            continue;
        }

        disp_cur_set(draw_y, draw_x);
        
        piece->draw();
    }

    cout << endl;

    cout << endl;

    cout << "Turn of player: " << player_turn << endl;

    cout << "Material for player 0: " << count_material(0) << endl;

}

enum winner Board::next_turn(bool original_call, int brute_force_depth, int material_extensions_left){

    int player = player_turn;
    player_turn = !player_turn;

    vector<board_move_t> best_moves = {};

#if REMEMBER_MOVES

    // see if this position has already been calculated before

    string current_state = get_state(player, additional_depth, extensions_performed);

    shared_lock lock_search(*already_calculated_moves_lock); // lock for reading

    auto it = already_calculated_moves->find(current_state);

    bool move_already_calculated = it != already_calculated_moves->end();

    lock_search.unlock();


    if(move_already_calculated){

        best_moves = it->second;

    }else
#endif
    {

        vector<board_move_t> all_valid_moves = {};

        for(Tile * tile : tiles){

            Piece * piece = tile->piece;
            if(!piece){
                continue;
            }

            if(piece->owner != player){
                continue;
            }

            vector<board_move_t> valid_moves = piece->get_valid_moves();

            if(valid_moves.size() <= 0){
                continue;
            }

            for(board_move_t move : valid_moves){
                all_valid_moves.push_back(move);
            }

        }

        if(all_valid_moves.size() <= 0){
            return WINNER_STALEMATE;
        }

        vector< pair< int , board_move_t > > move_evaluations (all_valid_moves.size());

        vector<thread> threads = {};

        ssize_t idx = -1;
        for(auto move : all_valid_moves){
            idx += 1;

            Board * imag_board = duplicate();

            auto fnc_imagine = [imag_board, brute_force_depth, idx, move, player, &move_evaluations, this, material_extensions_left]{

                enum winner imag_winner = imag_board->make_move(move);

                if(imag_winner == WINNER_NO_WINNER_YET){

                    if(brute_force_depth > 0){

                        imag_winner = imag_board->next_turn(false, brute_force_depth - 1, material_extensions_left);

                    }else if((material_extensions_left > 0) && (imag_board->count_material(player) > this->count_material(player))){

                        imag_winner = imag_board->next_turn(false, brute_force_depth, material_extensions_left - 1);

                    }

                }

                int material;

                switch(imag_winner){
                    case WINNER_NO_WINNER_YET:
                        material = imag_board->count_material(player);
                        break;
                    case WINNER_PLAYER_0:
                        material = -500 + (player == 0) * 1000;
                        break;
                    case WINNER_PLAYER_1:
                        material = -500 + (player == 1) * 1000;
                        break;
                    case WINNER_STALEMATE:
                        material = 0;
                        break;
                }

                move_evaluations.at(idx) = {material, move};

                delete imag_board;

            };

            if(original_call){

                // for some reason this started refusing to compile

                    // thread thr(fnc_imagine);

                    // threads.push_back(move(thr));
                    // // move the object here instead of copying because threads are not copy-able

                threads.push_back(thread(fnc_imagine));

            }else{

                fnc_imagine();

            }

        }

        for(auto & thr : threads){
            thr.join();
        }

        {
            int best_move_material = INT_MIN;

            for(auto [material, move] : move_evaluations){

                if(material > best_move_material){

                    best_move_material = material;

                    best_moves = {};
                    best_moves.push_back(move);
                    
                }else if(material == best_move_material){

                    best_moves.push_back(move);

                }

            }

            assert(best_move_material != INT_MIN);

#if REMEMBER_MOVES
            unique_lock lock_add(*already_calculated_moves_lock); // lock for writing

            (*already_calculated_moves)[current_state] = best_moves;

            lock_add.unlock();
#endif

        }

    }

    assert(best_moves.size() > 0);

    board_move_t move = vec_get_random_element(best_moves);

    return make_move(move);

}

int Board::count_material(int for_player){

    int count = 0;

    for(Tile * tile : tiles){

        if(!tile->piece){
            continue;
        }

        int value = 0;

        switch(tile->piece->type){
            case PT_PAWN:
                value = 1;
                break;
            case PT_KNIGHT:
                value = 4;
                break;
            case PT_BISHOP:
                value = 3;
                break;
            case PT_ROOK:
                value = 5;
                break;
            case PT_QUEEN:
                value = 9;
                break;
            case PT_KING:
                value = 200;
                break;
        }

        if(tile->piece->owner != for_player){
            value *= -1;
        }

        count += value;

    }

    return count;
}

pair<bool, Tile *> Board::get_tile_at(board_pos_t pos){

    auto [fail_ci, idx] = calc_idx(pos.first, pos.second);
    if(fail_ci){
        return {true, nullptr};
    }

    return {false, tiles[idx]};

}

enum winner Board::make_move(board_move_t move){

    enum winner winner = WINNER_NO_WINNER_YET;

    for(board_move_simple_t simple_move : move){

        auto [tile_from_fail, tile_from] = get_tile_at(simple_move.first);
        assert(!tile_from_fail);

        Piece * piece = tile_from->piece;
        assert(piece);

        enum winner win_tmp = piece->move_to(this, simple_move.second);

        if(winner == WINNER_NO_WINNER_YET){

            winner = win_tmp;

        }

    }

    return winner;

}

string Board::get_state(int arg_player_turn, int additional_depth, int extensions_performed){

    string state = "";

    assert(arg_player_turn <= 100);
    state += static_cast<char>(arg_player_turn);

    assert(additional_depth <= 100);
    state += static_cast<char>(additional_depth);

    assert(extensions_performed <= 50);
    state += static_cast<char>(extensions_performed);

    unsigned char repr = 0;
    bool repr_full = false;

    for(Tile * tile : tiles){

        Piece * piece = tile->piece;

        unsigned char rep;

        if(piece){
            rep = piece->get_representation();
        }else{
            rep = 0x0;
        }

        repr |= rep;

        if(repr_full){
            state += * reinterpret_cast<char*>(&repr);
        }else{
            repr = rep << 4;
        }

        repr_full = !repr_full;

    }

    assert(!repr_full);

    return state;

}

void Board::save_calculated_moves(string file){

#if USE_SAVE_FILE

    string file_tmp = file + "-tmp";

    ofstream tmp;
    tmp.open(file_tmp, ios::out | ios::binary);
    assert(tmp);

    shared_lock lock(*already_calculated_moves_lock); // lock for reading
    file_write___unsortedmap___string___vector__pair__pair_int__pair_int(tmp, already_calculated_moves);
    lock.unlock();

    filesystem::rename(file_tmp, file.c_str());
    // this CAN fail

#else

    (void)(file);

#endif

}

void Board::load_calculated_moves(string file){

#if USE_SAVE_FILE

    ifstream f;
    f.open(file, ios::in | ios::binary);
    if(!f){
        cout << "Could not open file `" << file << "`, library will not be loaded" << endl;
        return;
    }

    unique_lock lock(*already_calculated_moves_lock); // lock for writing
    file_read___unsortedmap___string___vector__pair__pair_int__pair_int(f, already_calculated_moves);
    lock.unlock();

#else

    (void)(file);

#endif

}

board_pos_t Board::terminal_position_to_board_position(pair<int, int> term_pos){

    {
        term_pos.first -= 1;

        if(term_pos.first % 2 != 0){
            return {-1, -1};
        }

        term_pos.first /= 2;
    }

    {
        term_pos.second -= 2;

        int rem = term_pos.second % 4;

        term_pos.second /= 4;

        if(rem == 0 || rem == 1){
            // no need for correction
        }else if(rem == 2){
            return {-1, -1};
        }else if(rem == 3){
            term_pos.second += 1;
        }else{
            UNREACHABLE();
        }
    }

    return term_pos;

}

// private

pair<bool, ssize_t> Board::calc_idx(int y, int x){
    if(y < 0 || y >= 8 || x < 0 || x >= 8){
        return {true, 0};
    }
    return {false, y * 8 + x};
}

void Board::set_tile_at(int y, int x, Tile * tile){

    auto [fail_ci, idx] = calc_idx(y, x);
    if(fail_ci){
        ERR("invalid tile y=" << y << " x=" << x);
    }

    if(tiles[idx]){
        ERR("tile not empty y=" << y << " x=" << x);
    }

    tiles[idx] = tile;

}

void Board::spawn_tiles(){

    if(tiles.size() != 0){
        ERR("already initialised");
    }

    for(int y=0; y<8; ++y){
        for(int x=0; x<8; ++x){

            Tile * tile = new Tile{
                .y = y,
                .x = x,
            };

            tiles.push_back(tile);

        }
    }

}

void Board::connect_neighbours(){

    for(Tile * tile : tiles){

        { // up
            int nei_up_y = tile->y - 1;
            int nei_up_x = tile->x;
            auto [nei_fail, nei] = get_tile_at({nei_up_y, nei_up_x});
            if(!nei_fail){
                tile->neighbour_up = nei;
            }
        }

        { // down
            int nei_up_y = tile->y + 1;
            int nei_up_x = tile->x;
            auto [nei_fail, nei] = get_tile_at({nei_up_y, nei_up_x});
            if(!nei_fail){
                tile->neighbour_down = nei;
            }
        }

        { // left
            int nei_up_y = tile->y;
            int nei_up_x = tile->x - 1;
            auto [nei_fail, nei] = get_tile_at({nei_up_y, nei_up_x});
            if(!nei_fail){
                tile->neighbour_left = nei;
            }
        }

        { // right
            int nei_up_y = tile->y;
            int nei_up_x = tile->x + 1;
            auto [nei_fail, nei] = get_tile_at({nei_up_y, nei_up_x});
            if(!nei_fail){
                tile->neighbour_right = nei;
            }
        }

        { // up left
            int nei_up_y = tile->y - 1;
            int nei_up_x = tile->x - 1;
            auto [nei_fail, nei] = get_tile_at({nei_up_y, nei_up_x});
            if(!nei_fail){
                tile->neighbour_upleft = nei;
            }
        }

        { // up right
            int nei_up_y = tile->y - 1;
            int nei_up_x = tile->x + 1;
            auto [nei_fail, nei] = get_tile_at({nei_up_y, nei_up_x});
            if(!nei_fail){
                tile->neighbour_upright = nei;
            }
        }

        { // down left
            int nei_up_y = tile->y + 1;
            int nei_up_x = tile->x - 1;
            auto [nei_fail, nei] = get_tile_at({nei_up_y, nei_up_x});
            if(!nei_fail){
                tile->neighbour_downleft = nei;
            }
        }

        { // down right
            int nei_up_y = tile->y + 1;
            int nei_up_x = tile->x + 1;
            auto [nei_fail, nei] = get_tile_at({nei_up_y, nei_up_x});
            if(!nei_fail){
                tile->neighbour_downright = nei;
            }
        }

    }

}

void Board::place_pieces(){

    for(auto [type, owner, forward_y, y, x_start, x_step] : {
        make_tuple(PT_PAWN,   1, 1, 1, 0, 1), make_tuple(PT_PAWN,   0, -1, 6, 0, 1),
        make_tuple(PT_ROOK,   1, 1, 0, 0, 7), make_tuple(PT_ROOK,   0, -1, 7, 0, 7),
        make_tuple(PT_KNIGHT, 1, 1, 0, 1, 5), make_tuple(PT_KNIGHT, 0, -1, 7, 1, 5),
        make_tuple(PT_BISHOP, 1, 1, 0, 2, 3), make_tuple(PT_BISHOP, 0, -1, 7, 2, 3),
        make_tuple(PT_QUEEN,  1, 1, 0, 3, 9), make_tuple(PT_QUEEN,  0, -1, 7, 3, 9),
        make_tuple(PT_KING,   1, 1, 0, 4, 9), make_tuple(PT_KING  , 0, -1, 7, 4, 9),
    }){

        for(int x=x_start; x<8; x+=x_step){

            auto [fail_ci, idx] = calc_idx(y, x);
            if(fail_ci){
                ERR("invalid position y=" << y << " x=" << x);
            }

            Tile * tile = tiles[idx];

            if(tile->piece){
                ERR("there is already a piece at y=" << y << " x=" << x);
            }

            Piece * piece = new Piece{
                .forward_y = forward_y,
                .owner = owner,
                .type = type,
                .location = tile,
            };

            tile->piece = piece;

        }

    }

}

///
//////
/////////// function: main
//////
///

int main(){

    Board * board = new Board;

    board->init();

    cout << "Loading moves db..." << endl;
    board->load_calculated_moves(SAVE_FILE);
    cout << "Loaded" << endl;

    enum winner winner = WINNER_NO_WINNER_YET;

    bool autopiot = false;

    while(winner == WINNER_NO_WINNER_YET){

        board->draw();

        cout << endl;

        string command;

        if(autopiot){
            command = "b";
        }else{
            cout << "Enter command: ";
            command = input_string();
        }

        if(command == "b"){

            winner = board->next_turn(true, BOT_BRUTE_FORCE_DEPTH, BOT_MAX_NUMBER_OF_MATERIAL_EXTENSIONS);

            if(rand() <= SAVE_CHANCE){
                cout << "Saving moves db..." << endl;
                board->save_calculated_moves(SAVE_FILE);
                cout << "Saved" << endl;
            }

        }else if(command == "h" || command == "c"){

            while(true){

                board_pos_t from = {};
                board_pos_t to = {};

                if(command == "h"){

                    from = input_chess_pos("Move from: ");
                    to = input_chess_pos("Move to: ");

                }else if(command == "c"){

                    cout << "Click on piece to move\r";
                    pair<int, int> from_unfiltered = input_mouse_click();
                    cout << "Click on destination  \r";
                    pair<int, int> to_unfiltered = input_mouse_click();
                    cout << "                    \r";

                    from = board->terminal_position_to_board_position(from_unfiltered);
                    to = board->terminal_position_to_board_position(to_unfiltered);

                }else{

                    UNREACHABLE();

                }

                auto [tile_fail, tile] = board->get_tile_at(from);
                if(tile_fail){
                    cout << "there is no such tile" << endl;
                    cout << "press enter" << endl;
                    input_enter();
                    break;
                }

                Piece * piece = tile->piece;
                if(!piece){
                    cout << "there is no piece on this tile" << endl;
                    cout << "press enter" << endl;
                    input_enter();
                    break;
                }

                if(piece->owner != board->player_turn){
                    cout << "piece is not controlled by the current player" << endl;
                    cout << "press enter" << endl;
                    input_enter();
                    break;
                }

                board_move_simple_t desired_move = {from, to};

                bool move_is_valid = false;

                vector<board_move_t> valid_moves = piece->get_valid_moves();

                board_move_t desired_move_fixed;

                for(board_move_t valid_move : valid_moves){
                    for(board_move_simple_t simple_move : valid_move){
                        // this means that if we are to do ambigious moves (like rokade)
                        // we need to move a certain piece (in rokade's case the king)
                        if(simple_move == desired_move){
                            move_is_valid = true;
                            desired_move_fixed = valid_move;
                            break;
                        }
                    }
                }

                if(!move_is_valid){
                    cout << "invalid move" << endl;
                    cout << "press enter" << endl;
                    input_enter();
                    break;
                }

                winner = board->make_move(desired_move_fixed);
                board->player_turn = !board->player_turn;

                break;

            }

        }else if(command == "auto"){

            autopiot = true;

        }else{

            cout << "Invalid command: `" << command << "`" << endl;
            cout << "PRESS ENTER";
            input_enter();

        }

    }

    delete board;

    switch(winner){
        case WINNER_NO_WINNER_YET:
            UNREACHABLE();
            break;
        case WINNER_PLAYER_0:
            cout << "Winner: player 0" << endl;
            break;
        case WINNER_PLAYER_1:
            cout << "Winner: player 1" << endl;
            break;
        case WINNER_STALEMATE:
            cout << "Stalemate" << endl;
            break;
    }

    return 0;
}
