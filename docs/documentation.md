# TKOM - projekt - dokumentacja wstępna
Jakub Proboszcz 318713

## 1. Funkcjonalność i przykłady kodu

### Typy wbudowane

Język obsługuje następujące wbudowane typy danych:

|   Typ | Opis                                                |
|-------|-----------------------------------------------------|
|   int | Reprezentuje liczbę całkowitą 32-bitową ze znakiem. |
| float | Reprezentuje liczbę zmiennoprzecinkową 64-bitową.   |
|  bool | Przyjmuje dwie wartości: `true` oraz `false`.       |
|   str | Reprezentuje ciąg 8-bitowych znaków.               |

Poza tym, użytkownik może definiować typy struktur i rekordów wariantowych.

### Deklaracje zmiennych
W języku zmienne są domyślnie stałe, tj. można im przypisać wartość tylko przy inicjalizacji.

Poniższa linijka deklaruje stałą typu int.
```
int a = 20;
a = 21; # błąd - a jest stałą
```

Żeby zadeklarować zmienną mutowalną, pomiędzy nazwą typu a nazwą zmiennej należy dodać znak `$`, jak poniżej.
```
int$ a = 20;
a = 21;
```
Deklaracja bez przypisania wartości jest niedozwolona.

Nie można przypisywać wartości zmiennym tymczasowym.
```
int a = 20;
a + 2 = 3; # błąd - przypisanie do zmiennej tymczasowej
```

### Obsługa typu string

Wyróżnikiem stringa jest znak `"`.

Szczególnie traktowane sekwencje znaków wewnątrz literału stringa są następujące:
| Sekwencja | Znaczenie |
|------|---------------------|
| `\t` | znak tabulacji |
| `\r` | Carriage Return |
| `\n` | Line Feed |
| `\"` | dosłowny `"` |
| `\\` | dosłowny `\` |
| `\x` | po sekwencji `\x` kolejne 2 znaki zostaną zinterpretowane jako heksadecymalna liczba określająca kod 8-bitowego znaku |

Literał stringa nie może zawierać `\`, poza powyższymi sekwencjami.\
Literał stringa nie może zawierać znaku nowej linii - zamiast niego można użyć `\n`.\
Wszystkie pozostałe znaki w literale stringa są traktowane dosłownie.

Przykłady:
```
str a = "abc\t\r\n\"\\\x00\xb7";

str b = "\k";     # błąd - nieznana sekwencja \k
str c = "\xg5";   # błąd - nieprawidłowa liczba heksadecymalna "g5"
str d = "a01"g5"; # błąd - " wewnątrz stringa musi być poprzedzony \
```

### Konwersje między typami
Język jest statycznie typowany - zmienna nie może zmienić typu, z jakim została zadeklarowana.

Język jest słabo typowany - wszystkie konwersje mogą zachodzić domyślnie przy przypisaniu do zmiennej danego typu, przy użyciu operatora oraz przy wywołaniu lub powrocie z funkcji.

Dozwolone są konwersje między dowolnymi typami wbudowanymi:
- `int` na `float` -  zamienia `int` na odpowiadającą wartość liczbową
- `float` na `int` - zaokrągla liczbę do najbliższej liczby całkowitej (w przypadku dwóch równoodległych liczb całkowitych, wybierana jest dalsza od zera); jeżeli liczba jest zbyt duża lub zbyt mała, żeby zmieściła się w typie `int`, lub jest to nieliczba, następuje błąd czasu wykonania
- `int` lub `float` na `bool` - zamienia 0 na `false` oraz dowolną inną liczbę na `true`
- `int` lub `float` na `str` - zamienia liczbę na napis tak, że odwrotna konwersja odtworzy oryginalną liczbę
- `bool` na `int` lub `float` - zamienia `false` na 0 oraz `true` na 1
- `bool` na `str` - zamienia `false` i `true` na napis "false" lub "true"
- `str` na `int` lub `float` - interpretuje string jako liczbę tak samo, jak w przypadku literałów*; jeżeli string nie reprezentuje liczby (w przypadku `int`, liczby całkowitej), następuje błąd czasu wykonania
- `str` na `bool` - zamienia pusty string na `false` oraz dowolny niepusty string na `true`

*akceptuje `-` na początku liczby; string z literałem liczby całkowitej może być skonwertowany na `float`; ignoruje białe znaki na początku lub na końcu stringa

Przykłady:
```
float a = " -42\r\n"; # a === -42.0
int b = -4.5;         # b === -5
str c = -42;          # c === "-42"
bool d = "false";     # d === true
int e = "4.2";        # błąd - string nie reprezentuje liczby całkowitej
```

Możliwe jest wykonanie konwersji explicite składnią taką, jak przy wywołaniu funkcji.
```
int a = int("42"); # a === 42
```

Nie są dozwolone konwersje z i na typy struktur.\
Nie są dozwolone konwersje z typów rekordów wariantowych.\
Możliwa jest konwersja typu na rekord wariantowy, jeżeli rekord wariantowy może zawierać ten typ.

### Komentarze
Język obsługuje komentarze - jako komentarz traktowany jest fragment kodu pomiędzy `#` a końcem linii.

### Operatory
Poniższa tabela prezentuje wspierane operatory. Wszystkie operatory, poza `(` `)` i `[` `]`, są dwuargumentowe, chyba że podano inaczej.
| Operator | Argumenty | Znaczenie |
|----------|-----------|-----------|
| `(` `)` | dowolne | zmienia priorytet operacji |
| `(` `)` | lewy funkcja, między nawiasami argumenty | wywołuje funkcję |
| `[` `]` | lewy `str`, między nawiasami `int` | dostęp do znaku w stringu (indeksowanie od 0) |
| `.` | lewy struktura lub rekord wariantowy, prawy pole | dostęp do pola |
| unarny `-` | `int` lub `float` | zmiana znaku liczby na przeciwny |
| `*` | `int` lub `float` | mnożenie |
| `**`, `/` | `float` | potęgowanie, dzielenie |
| `//`, `%` | `int` | operacje dzielenia całkowitego i modulo |
| `@` | lewy `str`, prawy `int` | zwielokrotnienie stringa |
| `!` | `str` | konkatenacja stringów |
| `+`, `-` | `int` lub `float` | dodawanie, odejmowanie |
| `and`, `or`, `xor`, unarny `not` | `bool` | operacje boolowskie |
| `<`, `>`, `<=`, `>=` | `int` lub `float` | porównania |
| `==`, `!=` | dowolne wbudowane | porównania (przez wartość) |
| `===`, `!==` | dowolne wbudowane | porównania (bez konwersji typów) |
| `is` | lewy dowolny, prawy typ | sprawdzenie typu |
| `=` | lewy zmienna, prawy dowolny | przypisanie |

`[` `]` dla drugiego argumentu ujemnego lub nie mniejszego niż długość stringa powoduje błąd czasu wykonania. Znaki kodowane w UTF-8 jako wiele 8-bitowych liczb są traktowane jako wiele oddzielnych znaki.

`/`, `//`, `%` dla drugiego argumentu 0 powodują błąd czasu wykonania.

Jeżeli lewy argument operacji potęgowania (operator `**`) jest ujemny, następuje błąd czasu wykonania.

`==`, `!=` dla typów wbudowanych próbują skonwertować argumenty do takich samych typów. Wykonywane są następujące konwersje, w zależności od typów argumentów:
| Argumenty | Typ docelowy |
|-----------|--------------|
| oba takie same | brak konwersji |
| `int` i `float` | `float` |
| `bool` i inny typ | ten inny typ |
| `str` i inny typ | `str` |

`===`, `!==` traktują wartości jako różne jeżeli argumenty są różnych typów.

`is` dla rekordów wariantowych zwraca `true`, jeżeli typ podany jako prawy argument jest typem aktualnie przechowywanym w rekordzie.

Operator `.` jest jedynym, którego można używać po lewej stronie przypisania.

Wszelkie przekroczenia zakresu zmiennych typu `int` powodują błąd czasu wykonania.

Operatory `==`, `!=` dla struktur wymagają tego samego typu dla obu argumentów i wykonują porównanie każdego elementu struktury  odpowiednim operatorem.

Operatory `==`, `!=` dla rekordów wariantowych wymagają tego samego typu dla obu argumentów i wykonują porównanie elementów zawartych w rekordach odpowiednim operatorem. Jeżeli typy zawarte w rekordach nie pozwalają na porównanie, zwracana jest wartość `false`.

Operatory przyjmujące typy `int` lub `float` zwracają typ `int` tylko, jeżeli oba ich argumenty są typu `int`; w przeciwnym razie zwracany jest typ `float`.
```
bool a = (2 + 2.3) is float; # a === true
bool b = (2.3 - 2) is float; # b === true
bool c = (2 * 3) is int;     # c === true
```

Priorytety (w kolejności od najwyższego) i łączności operatorów:
| Operatory | Łączność |
|---|---|
| `(` `)` | lewy-do-prawej |
| `.` | lewy-do-prawej |
| `[` `]` | lewy-do-prawej |
| `is` | nie dotyczy |
| unarne `-`, `not` | prawy-do-lewej |
| `**` | lewy-do-prawej |
| `*`, `/`, `//`, `%` | lewy-do-prawej |
| `+`, `-` | lewy-do-prawej |
| `<`, `>`, `<=`, `>=` | nie dotyczy |
| `@` | lewy-do-prawej |
| `!` | lewy-do-prawej |
| `==`, `!=`, `===`, `!==` | nie dotyczy |
| `and` | lewy-do-prawej |
| `xor` | lewy-do-prawej |
| `or` | lewy-do-prawej |
| `=` | nie dotyczy |

Operatory oznaczone w powyższej tabeli jako "nie dotyczy" nie dopuszczają użycia 2 lub więcej z nich jednego za drugim.

Przykłady:
```
int a = 2 / 2 * 3;       # a === 3
int b = "4" ** 2 ** "1"; # b === 16
str c = "4" ! "2" @ "2"; # c === "422"
float e = 2 / 4.0;       # e === 0.5
bool f = 3 === -"-3";    # f === false - unarny minus zwraca float gdy argument nie jest int
int g = "2345"["1"];     # g === 3
int h = "abc"["2.1"];    # błąd - string nie reprezentuje liczby całkowitej
bool a = 2 < 3 < 4;      # błąd - operatory porównania nie dopuszczają wielu porównań jedno za drugim
```
### Rekordy wariantowe
Język wspiera definiowanie przez użytkownika rekordów wariantowych:
```
variant Variant {
    int a;
    str b;
    bool c;
}
```
Określa to nowy typ danych, który pozwala przechowywać wartość jednego z wymienionych w definicji typów.

Przypisać wartość do rekordu wariantowego można poprzez przypisanie wartości do pola rekordu lub przypisanie wartości odpowiedniego typu bezpośrednio do rekordu. Przy inicjalizacji możliwa jest tylko opcja druga.

Dostęp do pól rekordu wariantowego w celu przypisania następuje poprzez operator `.`.
```
Variant$ a = 2; # a.a === 2
a.b = "string";
a.c = 3.4;      # domyślna konwersja na bool
a = true;       # a is bool
a = 3.4;        # błąd - nieprawidłowy typ przypisany do rekordu wariantowego
```
Można explicite skonwertować wartość odpowiedniego typu na typ rekordu wariantowego.
```
func f(int i) -> int { return 1; }
func f(Variant v) -> int { return 2; }

func main() {
    print(f(Variant(3))); # 2
}
```
Dostęp do wartości pola rekordu wariantowego przez operator `.` jest zabroniony (z wyjątkiem przypisania w instrukcji `if` z deklaracją).
```
Variant$ a = 2; # a.a === 2
bool b = a.c;   # błąd - dostęp do wartości rekordu wariantowego przez '.' poza 'if' z deklaracją
```
Można sprawdzić typ obecnie przechowywany w rekordzie wariantowym za pomocą operatora `is`.
```
if(a is bool) {
    # a przechowuje typ bool
}
```
Można dostać się do wartości przechowywanej w rekordzie wariantowym przez specjalną instrukcję `if` z deklaracją:
```
if(bool b = a) {}
if(bool b = a.c) {}
```
Po prawej stronie przypisania musi być rekord wariantowy, lub pole rekordu wariantowego o typie takim, jak typ deklarowanej zmiennej. Typ deklarowanej zmiennej musi być jednym z typów przechowywanych w rekordzie wariantowym.

Instrukcje w bloku instrukcji `if` wykonają się, jeżeli rekord wariantowy zawiera typ podany w deklaracji.

```
a.a = 2;
if(int value = a) {
    # przypadek int (obecne jest a.a)
}
elif(str value = a.b) {
    # przypadek str (obecne jest a.b)
}
elif(bool value = a) {
    # przypadek bool (obecne jest a.c)
}
```
Deklarowana zmienna może być oznaczona jako mutowalna - jej modyfikacja nie zmieni wartości w rekordzie wariantowym.
```
a.a = 2;
if(int$ value = a) {
    value = 3;
}
# a przechowuje typ int, wartość 2
```
Tak zadeklarowana zmienna jest widoczna tylko wewnątrz bloku instrukcji tej instrukcji `if`.
```
a.a = 2;
if(int$ value = a) {}
value = 3; # błąd - nieznana zmienna value
```

Typy pól rekordu wariantowego muszą być różne i nie mogą być oznaczone jako mutowalne - mutowalność wszystkich pól jest taka, jak całego rekordu.
```
variant Variant1 {
    int a;
    int b; # błąd - dwa pola rekordu wariantowego o tym samym typie
}
variant Variant2 {
    int$ a; # błąd - pole rekordu wariantowego oznaczone jako mutowalne
    str b;
}
```
Rekord wariantowy może mieć struktury i inne rekordy wariantowe jako pola.
```
variant Variant {
    int a;
    str b;
}
struct Struct {
    int a;
    str b;
}
variant StructOrVariant {
    Struct a;
    Variant b;
}
```
Rekord wariantowy nie może zawierać sam siebie, bezpośrednio lub pośrednio.
```
variant Wrong {
    int a;
    Wrong b; # błąd - rekord wariantowy zawiera sam siebie
}
```
```
struct Struct {
    int a;
    StructOrInt b;
}
variant StructOrInt {
    Struct a; # błąd - rekord wariantowy zawiera sam siebie
    int b;
}
```
Konwersje rekordów wariantowych na inne typy są niedozwolone.

Możliwe jest wywołanie funkcji przyjmującej wartość typu pola rekordu wariantowego bezpośrednio z rekordem wariantowym jako argumentem, jeżeli zdefiniowane są funkcje dla wszystkich pól tego rekordu wariantowego. Przy wyborze funkcji rekord wariantowy zostanie potraktowany tak, jak wartość obecnie w nim zawarta.
```
variant V {int a; str b; bool c;}
func f(int v) -> int { return 1; }
func f(str v) -> int { return 2; }
func f(bool v) -> int { return 3; }

func main() {
    V vart1 = 2;
    V vart2 = "string";
    print(f(vart1)); # 1
    print(f(vart2)); # 2
}
```
```
variant V {int a; str b; bool c;}
func f(int v) -> int { return 1; }
func f(str v) -> int { return 2; }

func main() {
    V vart1 = 2;
    print(f(vart1)); # błąd - niezdefiniowane funkcje dla wszystkich pól rekordu wariantowego
}
```
Jeżeli istnieje też funkcja przeciążona przyjmująca ten typ rekordu wariantowego, będzie ona preferowana.
```
variant V {int a; str b;}
func f(int v) -> int { return 1; }
func f(str v) -> int { return 2; }

func f(V v) -> int { return 3; }

func main() {
    V vart1 = 2;
    print(f(vart1)); # 3
}
```

### Struktury
Język wspiera definiowanie przez użytkownika struktur:
```
struct OtherStruct {
    int a;
    str b;
}
struct MyStruct {
    int a;
    float b;
    OtherStruct c;
    bool d;
}
```
Określa to nowy typ składający się z nazwanych pól dowolnego znanego typu.

Typy pól struktury nie mogą być oznaczone jako mutowalne - mutowalność wszystkich pól struktury jest taka, jak całej struktury.
```
struct Struct1 {
    int$ a; # błąd - pole struktury oznaczone jako mutowalne
}
```
Struktura nie może zawierać samej siebie, bezpośrednio lub pośrednio.
```
struct Struct1 {
    Struct1 a; # błąd - struktura zawiera samą siebie
}
```
```
struct Struct1 {
    Struct2 a;
}
struct Struct2 {
    Struct1 a; # błąd - struktura zawiera samą siebie
}
```

Inicjalizacja struktury następuje przez podanie wartości wszystkich pól w odpowiedniej kolejności, w nawiasach klamrowych `{` `}`.
```
MyStruct a = {2, 4.3, {4, "a"}, true};
```
Podczas przypisania do stałej lub zmiennej o typie struktury oraz podczas przypisywania wartości pola struktury niebędącego rekordem wariantowym w liście inicjalizacyjnej struktury nie trzeba podawać typu struktury.\
W pozostałych przypadkach listę inicjalizacyjną należy explicite skonwertować na typ struktury.
```
int a = MyStruct({2, 4.3, {4, "a"}, true}).a;
function_taking_MyStruct(MyStruct({2, 4.3, {4, "a"}, true}));
# nie potrzeba podawać typu OtherStruct
```
```
struct S1 {
    int a;
    int b;
}
variant V {
    S1 s;
    int i;
}
struct S2 {
    V v;
    int i;
}

func main() {
    S2 s = {S1({2, 3}), 2};
    # konieczne jest podanie typu S1, ponieważ jest wewnątrz rekordu wariantowego
}
```
Dostęp do pól struktury następuje poprzez operator `.`.
```
int b = a.c.a; # b === 4
```

### Instrukcja warunkowa
Język wspiera instrukcję warunkową `if`:
```
if(a == 3) # warunek
{
    # instrukcje
}
elif(int val = v) # warunek z deklaracją - v to rekord wariantowy
{
    # instrukcje
}
else
{
    # instrukcje
}
```
Sekcje `elif` oraz `else` są opcjonalne; sekcji `elif` może być wiele.

Jeżeli wartość w nawiasach skonwertowana na typ `bool` przyjmuje wartość `true`, zostanie wykonany blok instrukcji bezpośrednio po warunku. W przeciwnym przypadku, sprawdzony zostanie warunek przy kolejnym słowie kluczowym `elif`, itd. aż któryś warunek będzie spełniony. Jeżeli żaden z warunków przy `if` lub `elif` nie zostanie spełniony, zostanie wykonany blok instrukcji przy `else`.

### Instrukcje pętli
Język wspiera następujące rodzaje pętli:
```
while(a > 3) # warunek
{
    # instrukcje
}

do {
    # instrukcje
}
while(a > 3) # warunek
```

W obu przypadkach blok kodu będzie wykonywany dopóki spełniony jest warunek (wartość w nawiasach skonwertowana na typ `bool` przyjmuje wartość `true`). W pierwszym wariancie warunek jest sprawdzany przed każdym wykonaniem pętli, w drugim wariancie blok instrukcji wykona się raz przed pierwszym sprawdzeniem warunku.

```
int$ a = 1;
do {
    a = a + 1;
} while(false)

# a === 2
```
Dostępne są instrukcje `continue`, powodująca natychmiastowe przejście do końca obecnej iteracji najgłębszej obecnie iterowanej pętli, oraz `break`, natychmiast wychodząca z najgłębszej obecnie iterowanej pętli.

```
int$ a = 0;
int$ b = 0;
while(a < 10) {
    do {
        b = b + 1;
        if(a > 4) {
            continue;
        }
        a = a + 1;
    } while(b < 10)
    if(b == 13) {
        break;
    }
    a = a + 1;
}
# a === 8, b === 13
```
Instrukcje te nie są dopuszczalne poza pętlą.

### Funkcje
Użytkownik może definiować własne funkcje. Deklaracja funkcji wygląda następująco:
```
func funkcja(int a, str b) -> bool
{
    # instrukcje
    return true;
}
```
Jeżeli funkcja nie zwraca wartości, typ zwracany nie powinien być określony.
```
func funkcja(int$ a)
{
    # instrukcje
}
```
Wartość jest zwracana z funkcji przez słowo kluczowe `return`. Możliwe jest użycie bezargumentowego `return`, żeby wcześniej wyjść z funkcji niezwracającej wartości.
```
func funkcja(int$ a)
{
    if(a === 3) {
        return;
    }
    a = a + 1;
}

func main() {
    int$ a = 2;
    funkcja(a); print(a); # 3
    funkcja(a); print(a); # 3
}
```

Argumenty są domyślnie przekazywane przez referencję uniemożliwiającą ich modyfikację. Żeby umożliwić modyfikację, typy argumentów powinny zostać oznaczone jako mutowalne.
```
func f(int a, int$ b)
{
    # a = 3; # błąd - a jest niemutowalne
    b = 4;
}

func main() {
    int a = 1;
    int b = 1;
    func(a, b);
    # a === 1, b === 4
}
```

Możliwe jest rekurencyjne wywoływanie funkcji.
```
func factorial(int n) -> int {
    if(n == 0 or n == 1) {
        return 1;
    }
    else {
        return n * factorial(n - 1);
    }
}

func main() {
    int a = factorial(3); # a === 6
    int b = factorial(4); # b === 24
}
```
Liczba funkcji w stosie wywołań jest ograniczona - przekroczenie limitu powoduje błąd czasu wykonania.
```
func factorial(int n) -> int {
    return n * factorial(n - 1);
}

func main() {
    int a = factorial(3); # błąd - przekroczono limit wywołań funkcji
}
```

Możliwe jest przeciążanie funkcji - deklaracja wielu funkcji o tej samej nazwie, ale różniących się liczbą argumentów lub ich typami.

Wybór przeciążonej funkcji spośród funkcji o tej samej liczbie argumentów następuje na podstawie liczby argumentów o typach oczekiwanych przez funkcję.

```
func f(int a, str b) -> int { return 1; }
func f(int a, int b) -> int { return 2; }

func main() {
    int c = f(1, 2);   # c === 2
    # f(1, 2.5);       # błąd - niejednoznaczne wywołanie funkcji
}
```

Nie jest możliwe przeciążanie funkcji po mutowalności argumentu.

```
func f(int a) {}
func f(int$ a) {} # błąd - duplikat funkcji
```


### Widoczność zmiennych
Zmienna zadeklarowana w bloku oznaczonym `{` `}` jest widoczna tylko wewnątrz bloku, w którym została zadeklarowana.

```
if(true) {
    int a = 4;
}
int b = a + 2; # błąd - nieznana zmienna a
```

Nie można deklarować zmiennej, jeżeli istnieje już zmienna o tej samej nazwie, ani typu, jeżeli istnieje już funkcja lub typ o tej samej nazwie.
```
func f(int a) {
    str a = "a"; # błąd - istnieje już stała a
}
```
```
int a = 0;
if(a == 0) {
    int a = 1; # błąd - istnieje już stała a
}
```

### Funkcje wbudowane
```
no_arguments() -> int
```
Zwraca liczbę argumentów wywołania programu.
```
argument(int index) -> str
```
Zwraca argument wywołania programu o podanym indeksie (indeksowanie od 0). Jeżeli nie istnieje argument o takim indeksie, powoduje błąd czasu wykonania.
```
print(str message)
println(str message)
```
Wypisuje podanego stringa na wyjście standardowe. `println` dodaje znak nowej linii na końcu stringa.
```
input() -> str
```
Zwraca linię z wejścia standardowego (wszystkie znaki do znaku nowej linii włącznie).
```
input(int no_chars) -> str
```
Zwraca do num_chars 8-bitowych znaków z wejścia standardowego.
```
len(str string) -> int
```
Zwraca długość stringa.
```
abs(float value) -> float
abs(int value) -> int
```
Zwraca wartość bezwzględną podanej liczby. Jeżeli wykracza ona poza zakres typu, następuje błąd czasu wykonania.
```
max(float first, float second) -> float
max(int first, int second) -> int
min(float first, float second) -> float
min(int first, int second) -> int
```
Zwraca większą (`max`) lub mniejszą (`min`) z dwóch podanych wartości.

### Schemat programu
Program składa się z definicji typów (struktur, rekordów wariantowych) i funkcji. Pozostałe instrukcje są dozwolone tylko wewnątrz funkcji.

Definicje typów i funkcji są zabronione wewnątrz jakichkolwiek bloków.

Kod może być załadowany z dodatkowych plików instrukcją:
```
include "file";
```
umieszczoną poza wszelkimi blokami. Zachowuje się to analogicznie do wklejenia zawartości pliku w miejsce instrukcji `include`.

W przetworzonym kodzie musi znajdować się dokładnie jedna funkcja o nazwie `main`, nie przyjmująca argumentów. Wykonanie programu rozpoczyna się jej wywołaniem.

## 2. Gramatyka
### Reguły parsera (EBNF)
Poniższe produkcje zakładają, że parser nie otrzymuje tokenów komentarzy i białych znaków.
```
PROGRAM =       { TOP_STMT } ;

TOP_STMT =      INCLUDE_STMT
              | STRUCT_DECL
              | VARIANT_DECL
              | FUNCTION_DECL ;

INCLUDE_STMT =  'include', STRING_LITERAL ;

STRUCT_DECL =   'struct', DECL_BLOCK ;

VARIANT_DECL =  'variant', DECL_BLOCK ;

DECL_BLOCK =    IDENTIFIER, '{', FIELD_DECL, { FIELD_DECL } , '}' ;

FIELD_DECL =    TYPE_IDENT, IDENTIFIER, ';' ;

FUNCTION_DECL = 'func', IDENTIFIER, '(', [ PARAMETERS ], ')', [ '->', TYPE_IDENT ] , INSTR_BLOCK ;

PARAMETERS =    VARIABLE_DECL, { ',', VARIABLE_DECL } ;

VARIABLE_DECL = TYPE_IDENT, VAR_DECL_BODY ;

VAR_DECL_BODY = IDENTIFIER
              | '$', IDENTIFIER ;

INSTR_BLOCK =   '{', { INSTRUCTION } , '}' ;

INSTRUCTION =   IDENTIFIER, DECL_OR_ASSIGN_OR_FUNCALL, ';'
              | BUILTIN_TYPE, NO_TYPE_DECL
              | RETURN_STMT
              | 'continue', ';'
              | 'break', ';'
              | IF_STMT
              | WHILE_STMT
              | DO_WHILE_STMT ;

DECL_OR_ASSIGN_OR_FUNCALL =
                NO_TYPE_DECL
              | { '.', IDENTIFIER }, '=', EXPRESSION
              | '(', [ EXPRESSION, { ',', EXPRESSION } ] , ')' ;

BUILTIN_DECL =  BUILTIN_TYPE, NO_TYPE_DECL, ';' ;

NO_TYPE_DECL =  VAR_DECL_BODY, '=', EXPRESSION ;

RETURN_STMT =   'return', [ EXPRESSION ] , ';' ;

IF_STMT =       'if', '(', IF_CONDITION, ')', INSTR_BLOCK,
                { 'elif', '(', IF_CONDITION, ')', INSTR_BLOCK } ,
                [ 'else', INSTR_BLOCK ] ;

IF_CONDITION =  EXPRESSION
              | VARIABLE_DECL, '=', EXPRESSION ;

WHILE_STMT =    'while', '(', EXPRESSION, ')', INSTR_BLOCK ;

DO_WHILE_STMT = 'do', INSTR_BLOCK, 'while', '(', EXPRESSION, ')' ;

EXPRESSION =    XOR_EXPR, { 'or', XOR_EXPR } ;

XOR_EXPR =      AND_EXPR, { 'xor', AND_EXPR } ;

AND_EXPR =      EQUALITY_EXPR, { 'and', EQUALITY_EXPR } ;

EQUALITY_EXPR = CONCAT_EXPR, [ EQUALITY_OP, CONCAT_EXPR ] ;

EQUALITY_OP =   '=='
              | '!='
              | '==='
              | '!==' ;

CONCAT_EXPR =   STR_MUL_EXPR, { '!', STR_MUL_EXPR } ;

STR_MUL_EXPR =  COMPARE_EXPR, { '@', COMPARE_EXPR } ;

COMPARE_EXPR =  ADDITIVE_EXPR, [ COMPARISON_OP, ADDITIVE_EXPR ] ;

COMPARISON_OP = '>'
              | '<'
              | '>='
              | '<=' ;

ADDITIVE_EXPR = TERM, { ADDITIVE_OP, TERM } ;

ADDITIVE_OP =   '+'
              | '-' ;

TERM =          FACTOR, { MULTIPL_OP, FACTOR } ;

MULTIPL_OP =    '*'
              | '/'
              | '//'
              | '%' ;

FACTOR =        UNARY_EXPR, { '**', UNARY_EXPR } ;

UNARY_EXPR =    { UNARY_OP } , IS_EXPR ;

UNARY_OP =      '-'
              | 'not' ;

IS_EXPR =       SUBSCRPT_EXPR, [ 'is', TYPE_IDENT ] ;

SUBSCRPT_EXPR = DOT_EXPR, { '[', EXPRESSION, ']' } ;

DOT_EXPR =      STRUCT_EXPR, { '.', IDENTIFIER } ;

STRUCT_EXPR =   '{', EXPRESSION, { ',', EXPRESSION } , '}'
              | PARENTH_EXPR ;

PARENTH_EXPR =  IDENTIFIER, [ '(', [ EXPRESSION, { ',', EXPRESSION } ] , ')' ]
              | BUILTIN_TYPE, '(', EXPRESSION, ')'
              | '(', EXPRESSION, ')'
              | LITERAL ;

LITERAL =       STRING_LITERAL
              | INT_LITERAL
              | FLOAT_LITERAL
              | BOOL_LITERAL ;

BOOL_LITERAL =  'true'
              | 'false' ;

TYPE_IDENT =    BUILTIN_TYPE
              | IDENTIFIER ;

BUILTIN_TYPE =  'int'
              | 'float'
              | 'str'
              | 'bool' ;

```


### Reguły leksera (wyrażenia regularne)
```
IDENTIFIER
[[:alpha:]_][[:alnum:]_']*

[:alpha:] - dowolny znak, dla którego std::iswalpha zwraca true
[:alnum:] - dowolny znak, dla którego std::iswalnum zwraca true
```
Identyfikator nie może być słowem kluczowym. Maksymalna długość identyfikatora jest ograniczona.

```
STRING_LITERAL
"(\\t|\\r|\\n|\\"|\\\\|\\x[0-9a-fA-F]{2}|[^"\n\\])*"
```
Maksymalna długość literału stringa jest ograniczona.
```
INT_LITERAL
0|[1-9][0-9]*
```
Dozwolone są tylko wartości liczb całkowitych nie większe niż $2^{31}-1$. Sekwencje rozpoczynające się od znaku `-` są traktowane jako literał nieujemny i unarny operator `-`.
```
FLOAT_LITERAL
(0|[1-9][0-9]*)\.[0-9]*
```
Sekwencje rozpoczynające się od znaku `-` są traktowane jako literał nieujemny i unarny operator `-`.
```
COMMENT
#[^\n]*\n
```
Lekser emituje tokeny komentarzy; są one odrzucane przed przekazaniem do parsera. Maksymalna długość komentarza jest ograniczona.

Lekser nie emituje tokenów białych znaków. Białe znaki nie są ignorowane tylko przy oddzielaniu tokenów, lub wewnątrz literałów stringa lub komentarzy.

Poza powyższymi lekser emituje tokeny słów kluczowych:
```
'include'
'struct'
'variant'
'func'
'continue'
'break'
'return'
'if'
'elif'
'else'
'while'
'do'
'is'
'or'
'xor'
'and'
'not'
'int'
'float'
'bool'
'str'
'true'
'false'
```
oraz innych znaków sterujących:
```
'{'
'}'
';'
'('
')'
'->'
','
'$'
'='
'.'
'=='
'!='
'==='
'!=='
'!'
'@'
'>'
'<'
'>='
'<='
'+'
'-'
'*'
'/'
'//'
'%'
'**'
'['
']'
```
Rozmiar programu będzie ograniczony, w celu uniknięcia błędów interpretera związanych z przepełnieniem stosu, itp., które mogłyby wystąpić przy bardzo dużych programach.

## 3. Struktura programu
Interpreter jest pisany w języku C++.

W ogólności kod języka będzie przetwarzany kolejno przez następujące klasy:
- StreamReader - przyjmuje dowolny std::istream, leniwie produkuje kolejne znaki. Zamienia wszystkie sekwencje oznaczające koniec linii na pojedynczy znak `\n`. Rzuca wyjątek, jeżeli napotka znak kontrolny niebędący białym znakiem. Posiada metodę zwracającą kolejny znak z wejścia wraz z jego pozycją (numer linii i kolumny).
- Lexer - wykonuje analizę leksykalną, leniwie produkuje kolejne tokeny. Przyjmuje obiekt spełniający interfejs IReader; posiada metodę zwracającą kolejny token, wraz z jego pozycją w źródle.
- CommentDiscarder - przyjmuje obiekt spełniający interfejs ILexer, ze strumienia tokenów usuwa tokeny komentarzy.
- Parser - przyjmuje obiekt spełniający interfejs ILexer, ze strumienia tokenów tworzy drzewo składniowe. Klasy węzłów drzewa składniowego będą wspierać wzorzec wizytatora, w celu umożliwienia wypisania drzewa do stringa oraz dla dalszych elementów programu.
- SemanticAnalyzer - wizytator analizujący drzewo składniowe wyprodukowane przez Parser, sprawdza jego poprawność semantyczną oraz w razie potrzeby je modyfikuje, np. dodając instrukcje konwersji typów.
- Interpreter - wizytator przyjmujący drzewo składniowe oraz strumień wyjściowy programu, wykonuje program.

Wartości takie jak maksymalna długość identyfikatora lub stałej tekstowej, zakres typu `int` będą określone w kodzie, jako argumenty dla leksera.

### Obsługa błędów

Pierwszy napotkany błąd kończy przetwarzanie programu.

Błędy napotkane podczas analiz leksykalnej, składniowej i sematycznej, oraz przy generacji kodu powodują zakończenie działania programu przed wykonaniem jakiejkolwiek części programu.

Błędy czasu wykonania, napotkane podczas wykonywania kodu pośredniego, powodują przerwanie działania programu.

Przykładowy komunikat o błędzie:
```
Error: Expected '=', got ';'
at line 32, column 20 of input.
```

Przykładowy komunikat o błędzie czasu wykonania:
```
The program was terminated following a runtime error:
Recursion limit exceeded
while executing line 32 of input.
```

## 4. Sposób wywołania programu
`inter` to nazwa pliku wykonywalnego interpretera.

```
usage: inter [FILES] [--dump-dt|--args ARGS]
```
Wywołanie interpretera bezargumentowo powoduje załadowanie programu z wejścia standardowego. Interpreter nie jest interaktywny - przed wykonaniem programu wejście standardowe musi dobiec końca.

Wywołanie interpretera z argumentami załaduje kod programu z podanych plików zamiast z wejścia standardowego. Wywołanie takie jest równoważne wywołaniu z jednym plikiem wejściowym z instrukcjami `include` na początku ładującymi pozostałe pliki.

Wywołanie z opcją `--dump-dt` spowoduje wypisanie drzewa dokumentu programu na wyjście standardowe interpretera zamiast wykonania programu.

Wszystkie argumenty po opcji `--args` są traktowane jak argumenty wywołania interpretowanego programu.

## 5. Testowanie

Głównym sposobem testowania programu będą testy jednostkowe poszczególnych części programu.

Testy jednostkowe StreamReadera polegają na sprawdzeniu poprawnych konwersji różnych zakończeń linii oraz sprawdzaniu poprawności wyliczania pozycji znaków w źródle. Ponadto, sprawdzane jest poprawne rzucanie wyjątków dla niepoprawnych znaków lub przy błędzie strumienia wejściowego.

Testy jednostkowe Lexera będą polegać na sprawdzaniu poprawnej generacji różnego typu tokenów (min. jeden test na jeden token). Testowane będą błędne sekwencje dla każdego z tokenów: zbyt długi identyfikator, zbyt duży literał całkowity, zmiennoprzecinkowy i stringowy, zbyt długi komentarz. Ponadto, będą testowane również przypadki różnej ilości białych znaków między tokenami, np.:
```
abc+-abc
```
```
abc      +      -


abc
```
powinny wygenerować taką samą sekwencję tokenów:
- identyfikator `abc`
- operator `+`
- operator `-`
- identyfikator `abc`

Testy jednostkowe Parsera będą polegać na sprawdzeniu poprawności drzewa składniowego utworzonego na podstawie podanej sekwencji tokenów. Poprawność drzewa będzie weryfikowana przez wypisanie go do stringa i porównanie ze wzorcem. Dla każdej produkcji będzie co najmniej jeden test jednostkowy. Testowane będą przypadki typowych błędów programistów, np. użycie `=` zamiast `==`:
```
func main() {
    int a = 4;
    if(a = 4) {
        print("message");
    }
}
```
lub brak nawiasu klamrowego czy średnika:
```
func main() {
    int a = 4;
    if(a == 4) {
        print("message");
    }
```
```
func main() {
    int a = 3
    if(a == 4) {
        print("message");
    }
}
```

Testy jednostkowe SemanticAnalyzera będą polegać na sprawdzaniu poprawnego wstawiania operacji konwersji typów do drzewa składniowego i wykrywania błędów takich, jak:
- redeklaracja zmiennej
- modyfikacja stałej
- próba konwersji struktury na inny typ
- próba wstawienia nazwy typu tam, gdzie powinna być zmienna
- próba wykonania instrukcji `break` lub `continue` poza pętlą

i innych. Na każdy rodzaj błędu będzie min. 1 test jednostkowy.

Testy jednostkowe Interpretera będą polegały na sprawdzeniu zachowania interpretera dla różnych drzew dokumentu. Weryfikacja przypadków bez błędów będzie polegać na sprawdzeniu zawartości strumienia wyjściowego programu (w testach będzie to string). Będzie co najmniej 1 test jednostkowy na każdy rodzaj błędu czasu wykonania.

Ponadto, coraz większe części potoku przetwarzania (Lexer-Parser, Lexer-Parser-SemanticAnalyzer, itd.) będą weryfikowane testami integracyjnymi na przykładach z pierwszej sekcji dokumentacji wstępnej. Jeżeli w przykładzie podawane były instrukcje poza funkcją, zostaną one umieszczone w funkcji `main`, żeby utworzyć poprawny program.

Ponadto, będą przygotowane testy integracyjne całości skompilowanego programu dla kilku przygotowanych programów.
