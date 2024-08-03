
// TODO
//
// implementing stealmate by repeating turns
//
// rokada

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

#define ICON_PAWN_WHITE   "♙"
#define ICON_KNIGHT_WHITE "♘"
#define ICON_BISHOP_WHITE "♗"
#define ICON_ROOK_WHITE   "♖"
#define ICON_QUEEN_WHITE  "♕"
#define ICON_KING_WHITE   "♔"

#define ICON_PAWN_BLACK   "♟︎"
#define ICON_KNIGHT_BLACK "♞"
#define ICON_BISHOP_BLACK "♝"
#define ICON_ROOK_BLACK   "♜"
#define ICON_QUEEN_BLACK  "♛"
#define ICON_KING_BLACK   "♚"

#define ICON_WARNING "⚠"

///
//////
/////////// enum + declaration
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

class Tile;

class Board{

    public:

        vector<Tile *> tiles = {};

        int player_turn = 0; // which player's turn is it

        // Board();

        ~Board();

        void init();

        Board * duplicate();

        void draw();

        enum winner next_turn(int additional_depth);

        int count_material(int for_player);

        pair<bool, Tile *> get_tile_at(pair<int, int> pos);

        enum winner move_piece_to(pair<int, int> from, pair<int, int> to);

    private:

        pair<bool, ssize_t> calc_idx(int y, int x);

        void set_tile_at(int y, int x, Tile * tile);

        void spawn_tiles();

        void connect_neighbours();

        void place_pieces();
};

class Piece{

    public:

        string icon;

        int forward_y;
        // int forward_x = 0; // this only makes sense in 4way chess

        int owner; // which player owns the piece

        enum piece_type type;

        Tile * location;

        bool has_not_moved = true;

        Piece * duplicate(Tile * arg_location);

        vector<pair<int, int>> get_valid_moves();

        enum winner move_to(Board * board, pair<int, int> pos);

        pair<int, int> get_pos();

    private:

        vector<pair<int, int>> gen_valid_moves_pawn();

        vector<pair<int, int>> gen_valid_moves_knight();

        vector<pair<int, int>> gen_valid_moves_bishop();

        vector<pair<int, int>> gen_valid_moves_rook();

        vector<pair<int, int>> gen_valid_moves_king();

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

        pair<int, int> get_pos();

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

///
//////
/////////// module: terminal escape code
//////
///

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

    copy->icon = icon;

    copy->forward_y = forward_y;

    copy->owner = owner;

    copy->type = type;

    copy->location = arg_location;

    copy->has_not_moved = has_not_moved;

    return copy;

}

vector<pair<int, int>> Piece::get_valid_moves(){

    vector<pair<int, int>> moves = {};

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
            for(pair<int, int> rook_move : gen_valid_moves_rook()){
                moves.push_back(rook_move);
            }
        }break;

        case PT_KING:{
            moves = gen_valid_moves_king();
        }break;

    }

    return moves;

}

enum winner Piece::move_to(Board * board, pair<int, int> pos){

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

pair<int, int> Piece::get_pos(){

    return location->get_pos();

}

// private

vector<pair<int, int>> Piece::gen_valid_moves_pawn(){

    vector<pair<int, int>> moves = {};

    if(forward_y == -1){

        if(location->neighbour_up && !location->neighbour_up->piece){

            moves.push_back(location->neighbour_up->get_pos());

            if(has_not_moved && location->neighbour_up->neighbour_up && !location->neighbour_up->neighbour_up->piece){

                moves.push_back(location->neighbour_up->neighbour_up->get_pos());

            }

        }

        if(location->neighbour_upleft && (location->neighbour_upleft->piece && location->neighbour_upleft->piece->owner != owner)){
            moves.push_back(location->neighbour_upleft->get_pos());
        }

        if(location->neighbour_upright && (location->neighbour_upright->piece && location->neighbour_upright->piece->owner != owner)){
            moves.push_back(location->neighbour_upright->get_pos());
        }

        // TODO en passant

    }else if(forward_y == 1){

        if(location->neighbour_down && !location->neighbour_down->piece){

            moves.push_back(location->neighbour_down->get_pos());

            if(has_not_moved && location->neighbour_down->neighbour_down && !location->neighbour_down->neighbour_down->piece){

                moves.push_back(location->neighbour_down->neighbour_down->get_pos());

            }

        }

        if(location->neighbour_downleft && (location->neighbour_downleft->piece && location->neighbour_downleft->piece->owner != owner)){
            moves.push_back(location->neighbour_downleft->get_pos());
        }

        if(location->neighbour_downright && (location->neighbour_downright->piece && location->neighbour_downright->piece->owner != owner)){
            moves.push_back(location->neighbour_downright->get_pos());
        }

        // TODO en passant

    }else{

        UNREACHABLE();

    }

    return moves;

}

vector<pair<int, int>> Piece::gen_valid_moves_knight(){

    vector<pair<int, int>> moves = {};

    // TODO in the 4way chess map this won't be sufficient

    if(location->neighbour_up){

        if(location->neighbour_up->neighbour_upleft){
            if(!location->neighbour_up->neighbour_upleft->piece || (location->neighbour_up->neighbour_upleft->piece->owner != owner)){
                moves.push_back(location->neighbour_up->neighbour_upleft->get_pos());
            }
        }

        if(location->neighbour_up->neighbour_upright){
            if(!location->neighbour_up->neighbour_upright->piece || (location->neighbour_up->neighbour_upright->piece->owner != owner)){
                moves.push_back(location->neighbour_up->neighbour_upright->get_pos());
            }
        }

    }

    if(location->neighbour_down){

        if(location->neighbour_down->neighbour_downleft){
            if(!location->neighbour_down->neighbour_downleft->piece || (location->neighbour_down->neighbour_downleft->piece->owner != owner)){
                moves.push_back(location->neighbour_down->neighbour_downleft->get_pos());
            }
        }

        if(location->neighbour_down->neighbour_downright){
            if(!location->neighbour_down->neighbour_downright->piece || (location->neighbour_down->neighbour_downright->piece->owner != owner)){
                moves.push_back(location->neighbour_down->neighbour_downright->get_pos());
            }
        }

    }

    if(location->neighbour_left){

        if(location->neighbour_left->neighbour_upleft){
            if(!location->neighbour_left->neighbour_upleft->piece || (location->neighbour_left->neighbour_upleft->piece->owner != owner)){
                moves.push_back(location->neighbour_left->neighbour_upleft->get_pos());
            }
        }

        if(location->neighbour_left->neighbour_downright){
            if(!location->neighbour_left->neighbour_downright->piece || (location->neighbour_left->neighbour_downright->piece->owner != owner)){
                moves.push_back(location->neighbour_left->neighbour_downright->get_pos());
            }
        }

    }

    if(location->neighbour_right){

        if(location->neighbour_right->neighbour_upright){
            if(!location->neighbour_right->neighbour_upright->piece || (location->neighbour_right->neighbour_upright->piece->owner != owner)){
                moves.push_back(location->neighbour_right->neighbour_upright->get_pos());
            }
        }

        if(location->neighbour_right->neighbour_downright){
            if(!location->neighbour_right->neighbour_downright->piece || (location->neighbour_right->neighbour_downright->piece->owner != owner)){
                moves.push_back(location->neighbour_right->neighbour_downright->get_pos());
            }
        }

    }

    return moves;

}

vector<pair<int, int>> Piece::gen_valid_moves_bishop(){

    vector<pair<int, int>> moves = {};

    {
        Tile * pos = location;

        while(true){
            pos = pos->neighbour_upleft;
            if(!pos){
                break;
            }

            if(pos->piece){
                if(pos->piece->owner != owner){
                    moves.push_back(pos->get_pos());
                }

                break;
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
                    moves.push_back(pos->get_pos());
                }

                break;
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
                    moves.push_back(pos->get_pos());
                }

                break;
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
                    moves.push_back(pos->get_pos());
                }

                break;
            }
        }
    }

    return moves;

}

vector<pair<int, int>> Piece::gen_valid_moves_rook(){

    vector<pair<int, int>> moves = {};

    {
        Tile * pos = location;

        while(true){
            pos = pos->neighbour_up;
            if(!pos){
                break;
            }

            if(pos->piece){
                if(pos->piece->owner != owner){
                    moves.push_back(pos->get_pos());
                }

                break;
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
                    moves.push_back(pos->get_pos());
                }

                break;
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
                    moves.push_back(pos->get_pos());
                }

                break;
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
                    moves.push_back(pos->get_pos());
                }

                break;
            }
        }
    }

    return moves;

}

vector<pair<int, int>> Piece::gen_valid_moves_king(){

    vector<pair<int, int>> moves = {};

    if(location->neighbour_up && (!location->neighbour_up->piece || location->neighbour_up->piece->owner != owner)){
        moves.push_back(location->neighbour_up->get_pos());
    }

    if(location->neighbour_upright && (!location->neighbour_upright->piece || location->neighbour_upright->piece->owner != owner)){
        moves.push_back(location->neighbour_upright->get_pos());
    }

    if(location->neighbour_right && (!location->neighbour_right->piece || location->neighbour_right->piece->owner != owner)){
        moves.push_back(location->neighbour_right->get_pos());
    }

    if(location->neighbour_downright && (!location->neighbour_downright->piece || location->neighbour_downright->piece->owner != owner)){
        moves.push_back(location->neighbour_downright->get_pos());
    }

    if(location->neighbour_down && (!location->neighbour_down->piece || location->neighbour_down->piece->owner != owner)){
        moves.push_back(location->neighbour_down->get_pos());
    }

    if(location->neighbour_downleft && (!location->neighbour_downleft->piece || location->neighbour_downleft->piece->owner != owner)){
        moves.push_back(location->neighbour_downleft->get_pos());
    }

    if(location->neighbour_left && (!location->neighbour_left->piece || location->neighbour_left->piece->owner != owner)){
        moves.push_back(location->neighbour_left->get_pos());
    }

    if(location->neighbour_upleft && (!location->neighbour_upleft->piece || location->neighbour_upleft->piece->owner != owner)){
        moves.push_back(location->neighbour_upleft->get_pos());
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

pair<int, int> Tile::get_pos(){

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
        
        cout << piece->icon;
    }

    cout << endl;

    cout << endl;

    cout << "Material for player 0 (white): " << count_material(0) << endl;

}

enum winner Board::next_turn(int additional_depth){

    int player = player_turn;
    player_turn = !player_turn;

    vector< pair< pair<int, int> , pair<int, int> > > all_valid_moves = {};

    for(Tile * tile : tiles){

        Piece * piece = tile->piece;
        if(!piece){
            continue;
        }

        if(piece->owner != player){
            continue;
        }

        vector<pair<int, int>> valid_moves = piece->get_valid_moves();

        if(valid_moves.size() <= 0){
            continue;
        }

        pair<int, int> piece_pos = piece->get_pos();

        for(pair<int, int> move : valid_moves){
            all_valid_moves.push_back({piece_pos, move});
        }

    }

    if(all_valid_moves.size() <= 0){
        return WINNER_STALEMATE;
    }

    vector< tuple< int , pair<int,int> , pair<int,int> > > move_evaluations;

    for(auto [from, to] : all_valid_moves){

        Board * imag_board = duplicate();

        enum winner imag_winner = imag_board->move_piece_to(from, to);

        if(imag_winner == WINNER_NO_WINNER_YET){
            int depth = additional_depth - 1;
            if(depth >= 0){
                imag_board->next_turn(depth);
                // ignoring the return value here
            }
        }

        int material = imag_board->count_material(player);

        move_evaluations.push_back({material, from, to});

        delete imag_board;

    }

    {
        int best_move_material = INT_MIN;
        vector< pair< pair<int, int> , pair<int, int> > > best_moves = {};

        for(auto [material, from, to] : move_evaluations){

            if(material > best_move_material){

                best_move_material = material;

                best_moves = {};
                best_moves.push_back({from, to});
                
            }else if(material == best_move_material){

                best_moves.push_back({from, to});

            }

        }

        assert(best_move_material != INT_MIN);

        auto [from, to] = vec_get_random_element(best_moves);

        enum winner winner = move_piece_to(from, to);
        if(winner != WINNER_NO_WINNER_YET){
            return winner;
        }
    }

    return WINNER_NO_WINNER_YET;

}

int Board::count_material(int for_player){

    int count = 0;

    for(Tile * tile : tiles){

        if(!tile->piece){
            continue;
        }

        int value;

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

pair<bool, Tile *> Board::get_tile_at(pair<int, int> pos){

    auto [fail_ci, idx] = calc_idx(pos.first, pos.second);
    if(fail_ci){
        return {true, nullptr};
    }

    return {false, tiles[idx]};

}

enum winner Board::move_piece_to(pair<int, int> from, pair<int, int> to){

    auto [tile_from_fail, tile_from] = get_tile_at(from);
    assert(!tile_from_fail);

    Piece * piece = tile_from->piece;
    assert(piece);

    return piece->move_to(this, to);

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

    for(auto [icon, type, owner, forward_y, y, x_start, x_step] : {
        make_tuple(ICON_PAWN_BLACK,   PT_PAWN,   1, 1, 1, 0, 1), make_tuple(ICON_PAWN_WHITE,   PT_PAWN,   0, -1, 6, 0, 1),
        make_tuple(ICON_ROOK_BLACK,   PT_ROOK,   1, 1, 0, 0, 7), make_tuple(ICON_ROOK_WHITE,   PT_ROOK,   0, -1, 7, 0, 7),
        make_tuple(ICON_KNIGHT_BLACK, PT_KNIGHT, 1, 1, 0, 1, 5), make_tuple(ICON_KNIGHT_WHITE, PT_KNIGHT, 0, -1, 7, 1, 5),
        make_tuple(ICON_BISHOP_BLACK, PT_BISHOP, 1, 1, 0, 2, 3), make_tuple(ICON_BISHOP_WHITE, PT_BISHOP, 0, -1, 7, 2, 3),
        make_tuple(ICON_QUEEN_BLACK,  PT_QUEEN,  1, 1, 0, 3, 9), make_tuple(ICON_QUEEN_WHITE,  PT_QUEEN,  0, -1, 7, 3, 9),
        make_tuple(ICON_KING_BLACK,   PT_KING,   1, 1, 0, 4, 9), make_tuple(ICON_KING_WHITE,   PT_KING  , 0, -1, 7, 4, 9),
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
                .icon = icon,
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

    enum winner winner = WINNER_NO_WINNER_YET;

    while(winner == WINNER_NO_WINNER_YET){

        board->draw();
        cout << endl;

        cout << "Enter command: ";
        string command = input_string();

        if(command == "b"){

            winner = board->next_turn(3);

        }else if(command == "h"){

            cout << "From y: ";
            int f0 = input_int();
            cout << "From x: ";
            int f1 = input_int();
            cout << "To y: ";
            int t0 = input_int();
            cout << "To x: ";
            int t1 = input_int();

            pair<int, int> from = {f0, f1};
            pair<int, int> to = {t0, t1};

            board->move_piece_to(from, to);
            board->player_turn = !board->player_turn;

        }else{

            cout << "Invalid command: `" << command << "`" << endl;
            input_enter();

        }

    }

    delete board;

    switch(winner){
        case WINNER_NO_WINNER_YET:
            UNREACHABLE();
            break;
        case WINNER_PLAYER_0:
            cout << "Winner: player 0 (white)" << endl;
            break;
        case WINNER_PLAYER_1:
            cout << "Winner: player 1 (black)" << endl;
            break;
        case WINNER_STALEMATE:
            cout << "Stalemate" << endl;
            break;
    }

    return 0;
}
