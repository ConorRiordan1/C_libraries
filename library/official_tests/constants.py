import socket

# change these values via CLI args
IP_ADDRESS = "127.0.0.1"
PORT = 8000
IP_DOMAIN = socket.AF_INET
ENCRYPTED = False
TIMEOUT = None

# note, should run the student's program with "random map"
# if there is time
MAP_PATH = "official_tests/maps/large_map"

BUF_SIZE = 16384

RETURN_CODES = {
    "success": 0x00,
    "credentials": 0x01,
    "session_id": 0x02,
    "not_found": 0x03,
    "permissions": 0x04,
    "invalid": 0x05,
    "unreachable": 0x06,
    "unknown": 0xFF,
}

OP_CODES = {
    "login": 0x01,
    "register": 0x02,
    "admin_add": 0x03,
    "user_del": 0x04,
    "user_pass": 0x05,
    "list": 0x06,
    "search": 0x07,
    "route": 0x08,
    "link_add": 0x09,
    "link_del": 0x0A,
    "location_add": 0x0B,
    "location_del": 0x0C,
}

SINGLE_ARG_OPS = ["user_del", "list", "search", "location_add", "location_del"]
DUAL_ARG_OPS = ["login", "register", "admin_add", "user_pass", "list", "route", "link_add", "link_del"]
ADMIN_OPS = ["admin_add", "user_del", "user_pass", "link_add", "link_del", "location_add", "location_del"]

CREDENTIALS = {
    "Alice": "EiEXwW4",
    "Bob": "H9uAZOi",
    "Charli3": "knd5FL",
    "D4n": "9Ll64ZB7",
    "Eve": "tmkNMj",
    "Frank": "LNlvprvw",
    "Gina": "qgJ8DhPlRk",
    "Hank": "YYVkZaLv",
    "Ivy2": "AZ8EWCnqS",
    "Jack3": "VglrgThd",
    "Kira": "<4cwbmax^<B",
    "Leo11": "}6i7:K5P42j",
    "Mia": '/H"w6`@|Rt',
    "Nico99": 'w#<-RH"M2Uo9',
    "Olivia": "./(#W\@fu|d",
    "Pete": '-FTrQ4"{;pcZG',
    "Quinn": "QJ9\s_4S]Ww@6S",
    "Rose": "pmi\g7lELD)C0",
    "Steve": "sg_/pG7.W/on#",
    "Tina56": "TlIrh;V-s62Q\_",
    "Uma": "ZW:z$H%&~#_;=$",
    "Victor32": "E{~wEyO^\se8l",
    "Wendy1": "8)gd`?&bK/J-",
    "Xander": "yI6USQbH)l%j^",
    "Yolanda": "!gJqfcT}rgc^P",
    "Z4ck9": "O\pnrdBfp#.Pn",
    "dppK": 'h*0i"Dt=WN)ow5nR',
    "Jlgip": "21DMQ1:!aC",
    "Dskdj": "b3lp2@rz;`?wr7i+",
    "lOTBNZxz": "Bk~_ch2IwwByc",
    "JroKAML": "p7qeM!.*sP-_i",
    "ngbCcnsA": "6lokrq[$dh;tuXA",
    "fynlZbY": "5fh]r(hYge",
    "zCqORX": "Z.!gvbf7fmcoq#Wb",
    "pXdfUl": "$BTufjo6ao",
    "hvXhaakC": "eSCE2=r?3P`*",
    "yYnfS": "y~f&tuhdz6",
    "tdksWTFS": "+3RC3-{kQePW",
    "mjUcg": "8HbR]Yd3;i",
    "FEjtLNb": "P;@4IJk<MdK`YIIN",
    "rMlTOvf": "]\\xgW*xlx8\-/c",
    "fymX": "3X:p/'Yy",
    "BQavsSLc": '*$H|H^rB/"vFf1',
    "vgpiMipD": "0Z9Ac*T,:",
    "cweVcPn": "3]&(i)0[Qeu",
    "aMTtuS": "`VB9_xt(7'wCNnw",
    "gCsP": "RxJmk^:9",
    "JRbyFJ": "ZPm>ax,$",
    "BKRfjQp": "%;?7'#/fTPd4k",
    "RIgE": "ip+$8m.[!.92",
    "LLMZx": "@)rHO8]XE}H^M{qd",
    "wTkLKGzC": "=%S)7*5ZZ$[F",
    "MPifNyJv": "$,,'e/3?!PThr",
    "lZWDG": "H)]$hs,i%_",
    "iwRd": ".6e:'rq(!G",
    "faonfU": "cfr1!0cu5J",
    "clhcRPV": "?ue~g/STQd",
    "scioEVzC": "}E>R,)T<edS",
    "aocam": "{..DvSmf#1%$",
    "TXRc": '?2rh(IV%D=nK"',
    "TVVW": "i|oA?1I(Y",
    "IGSLuZlc": "q~}lTajy9H?",
    "QbOJ": "o)pM$6;^N|`W/q",
    "VNUkBj": "poI7QD:'EYp\cPNK",
    "Pwsq": "?vB`r[!V*2!)CCaS",
    "IEbUjdOl": 'U>x[9ENU8>Dw"',
    "EPRx": "fgT:#!MjEhwX_",
    "MMHgtFKP": "kTi$2A96k!cwyN",
    "dLmJOr": "Dx'cb-019",
    "IABE": "~7%M$xGfC5h>M1",
    "MBZT": '=4IIFD4B!="',
    "ozBwO": 'T8J:K~}Pc"luo=',
    "ZpKKoeP": "^V>QeFt+@;O",
    "caYyqdjB": "@j-FO|6+)aqmZ",
    "DdGYvkH": "R\oyVw49L-",
    "uVyZizhF": "LPDja7^8o24m",
    "KhhGl": "xl/Rlt@yo:kb_7",
    "gGCnLF": "w2\Q2&wOr`>k.Cv)",
    "zpYc": "vE/$L[6A64a",
    "EueAhu": "xY*|&v]GbWd`7",
    "zLxxXea": '-"|6-^5e_v=<[',
    "xHVzLbOh": "D~1$O'uT&RVN\"R@b",
    "nAMFWT": "$nGd!GaQiu|",
    "FpoT": "Y*qmdd-adS=",
    "PSwyWyOt": "1<;-;=jCG",
    "RKCqU": "qS\VoUO|fzCT",
    "WyZteL": 'Yec3YF:"gUv}',
    "aGNPB": "d_/@0H*#",
    "nLYE": "UnDYlyC}eHh",
    "pZVKmU": "BEs|+Sh9[f&",
    "Abow": "Hq|Elw]$^",
    "wvWvk": 'V2^*"5!Q)V>JE`',
    "wdlD": '@E"|0Ix@Z+3',
    "ZIuLN": "st1vk6~v>A]Gj{",
    "DNKiqqj": ']Xx5_^T"xv2(B4=0',
    "mDBPHPB": "kZkT;^hy_j",
    "EimL": "]Xn6nf2wOr?",
    "EbqK": "y~Z9{1Pfl",
    "yhZCpbjk": "d%YbyX_JsH1",
    "sYNuKjU": "N@CN$~6h9",
}