#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/times.h>
#include <unistd.h>
#include <stdbool.h>
#include <dlfcn.h>
#include "../Person.h"
#include "../ContactBook.h"
#include "../ContactTree.h"

#define CLK sysconf(_SC_CLK_TCK)

void printTimes(struct tms *previousTms, clock_t *previousClock, bool start) {
    struct tms currentTms;
    times(&currentTms);
    clock_t currentClock = clock();

    if(!start){
        printf("Real time:\t%.6f\n",((double) (currentClock - *(previousClock))) / CLOCKS_PER_SEC );
        printf("User time:\t%.6f\n",((double) (currentTms.tms_stime - previousTms->tms_stime)) / CLK);
        printf("System time:\t%.6f\n",((double) (currentTms.tms_utime - previousTms->tms_utime)) / CLK );
    }

    *previousTms = currentTms;
    *previousClock = currentClock;

}

int main(){
    char* names[] = {"Dante","Galvin","Ayanna","Bryar","Alexander","Zelda","Hayes","Robert","Flynn","Nola","Jelani",
                     "Benedict","Nathaniel","Meghan","Adrian","Alice","Iris","Zorita","Silas","Marah","Adam","Melissa",
                     "Nora","Nash","Yvette","Kane","Justin","Pearl"};
    int IDs[] = {1147, 4353, 7398, 8292, 8347, 8732, 7029, 7400, 3991, 1310, 9794, 1649, 3546, 765, 9300, 2, 8065, 9475,
                 5222, 9981, 3724, 9964, 9349, 1461, 6502, 6005, 6039, 9746, 498, 1071, 2522, 78, 6113, 1778, 9895, 1808,
                 307, 5239, 5291, 1890, 2114, 9175, 3372, 1748, 7366, 399, 2216, 3207, 8068, 2549, 3792, 2531, 9307, 3462,
                 6873, 77, 7170, 5277, 3390, 6311, 991, 6145, 3677, 5699, 5600, 2751, 5243, 2678, 313, 5434, 1636, 5293, 1630,
                 8248, 1813, 6723, 9282, 1487, 3884, 879, 7567, 9085, 7873, 4832, 4093, 3206, 5998, 4626, 9296, 2035, 7453,
                 7962, 9209, 4601, 3491, 8752, 674, 1864, 37, 5525, 9644, 6210, 3573, 2575, 5751, 500, 2816, 4026, 817, 7353,
                 8431, 4908, 27, 1882, 902, 7758, 6387, 5938, 4777, 7388, 2271, 644, 9839, 6543, 9060, 3510, 7267, 2306, 8660,
                 4774, 3418, 1070, 2864, 4525, 9697, 800, 1773, 6768, 3596, 219, 5551, 2431, 8348, 3826, 287, 4726, 5559, 3697,
                 2818, 1940, 2689, 9374, 6426, 8289, 957, 2290, 1683, 7328, 2335, 1902, 1715, 9707, 1656, 7512, 3819, 6913, 4135,
                 2149, 7399, 6004, 9217, 2615, 7394, 1854, 2542, 2048, 963, 7253, 8233, 4051, 1072, 1909, 5778, 2072, 6149,
                 665, 5893, 159, 1631, 103, 5125, 3402, 4154, 1557, 6381, 2762, 2278, 4518, 1171, 3641, 760, 5521, 721, 8871,
                 8972, 6955, 3256, 9086, 2310, 9659, 2412, 2639, 9450, 8072, 8644, 4297, 8343, 6761, 1834, 9385, 4406, 8578,
                 7050, 88, 2836, 73, 9058, 3515, 4208, 488, 5991, 2356, 7995, 9711, 109, 6654, 4689, 1278, 4068, 9489, 8773,
                 6162, 1293, 4869, 8888, 6655, 1536, 2931, 8718, 5349, 6999, 9456, 1143, 4413, 5238, 980, 4998, 3031, 2960,
                 2153, 673, 5107, 3259, 1685, 8812, 8982, 1757, 4393, 5526, 6270, 5684, 714, 8273, 3733, 5325, 5026, 7690, 9793, 6675, 8375, 3445, 5642, 8179, 7159, 4802, 2405, 9136, 8084, 1418, 86, 6631, 2040, 3847, 1572, 6198, 3975, 1434, 8695, 4168, 5730, 7763, 4757, 1126, 7114, 7016, 6656, 6817, 8917, 5218, 3780, 601, 4450, 3229, 8177, 4771, 8902, 1857, 7748, 3446, 1779, 6446, 767, 7417, 5558, 335, 2441, 8935, 5955, 8768, 8131, 7004, 1200, 3511, 2129, 239, 4142, 3220, 1259, 1131, 6765, 6815,
                 3632, 9705, 8619, 8844, 4904, 664, 7874, 8284, 9002, 236, 5624, 7483, 3528, 2197, 1020, 5337, 6897, 6234, 6390, 9393, 6325, 7440, 6686,
                 6659, 5969, 755, 5697, 7917, 6594, 1378, 6125, 2066, 400, 2624, 8303, 8485, 7486, 9272, 2144, 8708, 1286, 356, 8810, 4698, 6711, 1389, 4503, 1596, 3674, 2982, 785, 8209, 7563, 7347, 9345, 7018, 8358, 7481, 8913, 8789, 8471, 381, 861, 200, 6937, 6690, 1583, 1347, 7051, 2090, 4136, 3814, 1663, 2123, 8470, 9780, 5269, 4644, 3430, 7799, 4345, 1830, 3767, 4576, 7068, 2554, 5906, 5281, 3479, 9216, 4555, 8409, 1283, 2969, 447, 6187, 9454, 8694, 2143, 3985, 94, 1720, 784, 3535, 4424, 7849, 4981, 380, 3896, 3033, 8218, 5901, 581, 4914, 7262, 1803, 8507, 6412,
                 2914, 7083, 6574, 8010, 5812, 6317, 8683, 9195, 7936, 3197, 6926, 6807, 8766, 9712, 3992, 2921, 6402, 4456, 7694, 4200, 4028, 2403, 1265, 7534, 6419, 89, 2917, 5405, 2502, 4366, 6945, 6740, 8886, 9927, 7714, 9841, 2631, 2997,
                 8776, 5324, 5157, 6963, 3850, 6664, 9729, 9768, 3318, 2070, 9520, 2328, 2202, 8442, 8864, 9678, 244, 8764, 9154, 5230, 1492, 2225, 6205, 2156, 4643, 5244, 8827, 7595, 7420, 7215, 8098, 6150, 3870, 7913, 217, 4376, 4167, 7790, 7487, 8924, 9284, 1534, 8730,
                 8012, 385, 4236, 3868, 1754, 9125, 9735, 6090, 158, 3534, 7518, 4284, 3451, 396, 3191, 3951, 7797, 1726, 4269, 6360, 7785, 9742, 7541, 2420, 9259, 1632, 4759, 7239, 1209, 6256, 3696, 1410, 2417, 6931, 7043, 8483, 5965, 6430, 1831, 9358, 1459, 6884, 4857, 6498, 4392, 8300, 4758, 4128, 4595, 1111, 2760, 5389, 366, 1597, 1767, 6010, 5098, 5672, 3416, 1472, 4878, 8777, 230, 6779, 1287, 104, 1014,
                 5541, 3394, 7318, 9381, 6741, 9176, 377, 1769, 6343, 5410, 5693, 4932, 9503, 526, 8073, 3524, 4715, 2735, 8396, 7343, 6353, 7233, 4963, 4286, 430, 4918, 2591, 2305, 6393, 9224, 1355, 6015, 3127, 1395, 8001, 1540, 4900, 8110, 3574, 1530, 294, 3346, 3139, 4005, 5510, 1359, 2419, 4863, 4197, 4958, 2231, 1895, 5599, 9834, 9018, 3526, 3237, 8600, 9401, 8064, 1790, 8603, 6597, 2536, 4811, 6861, 5297, 6695, 836, 3926, 9338, 6332, 9392, 8344, 2863, 1908, 8579, 5406, 3284, 6601, 7726, 7136, 2540, 8388, 7932, 1102, 25, 3023, 3449, 8413, 7311, 6982, 7658, 2049, 4479, 9121, 6483, 2351, 1661, 8823, 8845, 9064, 6967, 3419, 9703, 1601, 2187, 9898, 5857, 2465, 7500, 6354, 6549, 5717, 797, 5703, 8113, 2981, 4539, 4077, 3155, 4057, 8474, 5448, 883, 5093, 5862, 5548, 3497, 9043, 9627, 8247, 7904, 2532, 2641, 2073, 5173, 780, 9133, 282, 2857, 1059, 8903,
                 7912, 1694, 30, 1575, 2663, 4700, 1526, 1756, 9115, 8324, 5151, 7306, 531, 4519, 8352, 3624, 5580, 9337, 3825, 2071, 7914, 7354, 3471, 1873, 962, 2346, 3506, 6348, 6328, 3748, 3064, 9763, 1087, 4280, 8573, 5421, 5260, 3705, 7835, 8657, 843, 293, 7124, 7154, 5315, 8517, 6712, 6663, 6443, 6411, 2172, 9861, 2381, 3876,
                 4383, 3212, 3566, 9351, 5176, 3964, 9059, 2957, 4783, 3074, 4017, 1232, 461, 111, 2118, 2786, 4995, 1618, 3303, 3631, 5682, 3638, 9994, 5403, 179, 7455, 2026, 1935, 9616, 1659, 4624, 4527, 8798, 9552, 7490, 4729, 9458, 7759, 7219, 551, 1139, 5252, 4140, 2097, 5860, 9487, 3560, 9231, 5109, 5733, 6620, 1792, 3909, 959, 7208, 3727, 2433, 2289, 587, 2896, 5450, 4615, 4590, 9824, 3327, 6973, 7189, 60, 7536, 764, 6375, 7201, 5967, 6421, 3208, 781, 2361, 84, 3106, 1065, 53, 715, 6242, 6590, 2877, 4790, 5308, 3428, 935, 8227, 8164, 603, 2838, 1723, 7766,
                 285, 3498, 6445, 1046, 8108, 7738, 4454, 5331, 5236, 885, 3544, 4240, 1669, 3111, 79, 4670, 220, 3969, 382, 9277, 5944, 1833, 9212, 341, 5135, 6830, 7698, 7651, 7348, 5012, 5007, 5933, 8325, 3196, 973, 3463, 3067, 9128, 4613, 6231, 7351, 8221, 8849, 3102, 5000, 2012, 2952, 7528, 9753, 7853, 145, 8842,
                 3802, 8293, 6746, 9788, 7308, 4231, 8030, 2398, 9321, 3867, 1341, 9395, 7810, 166, 7934, 7142, 9622, 7571, 2686, 9295, 2323, 1704, 2519, 6350, 5160, 9513, 9324, 5804, 8309, 3287, 3738, 7922, 8153,
                 4674, 9905, 1966, 3875, 9809, 3490, 4515, 5647, 809, 196, 6003, 3621, 2966, 1328, 6448, 8070

    };

    void* library = dlopen("./libshared.so",RTLD_LAZY);
    if(!library) {
        return 1;
    }

    struct tms previousTms;
    clock_t previousClock;
    printTimes(&previousTms, &previousClock, true);
    printf("++++++++++++++++++++++++++++++++++++++++++++++++++++\n");
    printf("LIST\n");
    printf("++++++++++++++++++++++++++++++++++++++++++++++++++++\n");
    printf("Create list book:\n");

    struct Contact* (*createBook)() = dlsym(library, "createBook");

    struct Person* (*newPerson)(int id, char* name) = dlsym(library, "newPerson");

    struct Contact* (*pushPerson)(Contact* head, Person* p) = dlsym(library, "pushPerson");

    printTimes(&previousTms, &previousClock,false);
    printf("Add 1000 contacts\n");
    int i = 0;

    Contact * head = createBook();

    while(i < 1000){
        int r = rand() % 28;
        Person * p = newPerson(IDs[i], names[r]);
        head = pushPerson(head, p);
        i++;
    }
    printTimes(&previousTms, &previousClock,false);

    struct Contact* (*sort)(Contact * head) = dlsym(library, "sort");

    printf("Sort 1000 contacts\n");
    head = sort(head);
    printTimes(&previousTms, &previousClock,false);

    struct Contact* (*deleteContact) (Contact* head, Contact* c) = dlsym(library, "deleteContact");

    printf("Delete contact - optymistic\n");
    head = deleteContact(head,head);
    printTimes(&previousTms, &previousClock,false);

    struct Contact* (*deleteTail) (Contact* head) = dlsym(library, "deleteTail");

    printf("Delete contact - pesymistic\n");
    head = deleteTail(head);
    printTimes(&previousTms, &previousClock,false);

    struct Contact* (*deleteBook)(Contact *head) = dlsym(library, "deleteBook");

    printf("Delete book\n");
    deleteBook(head);
    printTimes(&previousTms, &previousClock,false);

    printf("++++++++++++++++++++++++++++++++++++++++++++++++++++\n");
    printf("TREE\n");
    printf("++++++++++++++++++++++++++++++++++++++++++++++++++++\n");

    struct ContactTree* (*createTree)() = dlsym(library, "createTree");

    struct ContactTree* (*pushPersonByID)(ContactTree* root, Person* p) = dlsym(library, "pushPersonByID");


    printf("Create tree book:\n");
    ContactTree * root = createTree();
    printTimes(&previousTms, &previousClock,false);

    printf("Add 1000 contacts\n");
    i = 0;

    while(i < 1000){
        int r = rand() % 28;
        Person * p = newPerson(IDs[i], names[r]);
        root = pushPersonByID(root, p);
        i++;
    }
    printTimes(&previousTms, &previousClock,false);

    struct ContactTree* (*sortByName)(ContactTree* root) = dlsym(library, "sortByName");

    printf("Sort 1000 contacts\n");
    root = sortByName(root);
    printTimes(&previousTms, &previousClock,false);

    struct ContactTree * (*maxName)(ContactTree * root) = dlsym(library, "findMax");

    printf("Find max\n");
    maxName(root);
    printTimes(&previousTms, &previousClock,false);

    struct ContactTree* (*deleteContactTree)(ContactTree* c, bool toFree) = dlsym(library, "deleteContactTree");

    printf("Delete contact - no kids\n");
    root = deleteContactTree(maxName(root), true);
    printTimes(&previousTms, &previousClock,false);

    printf("Delete contact - with kids\n");
    deleteContactTree(root, true);
    printTimes(&previousTms, &previousClock,false);

    struct ContactTree* (*deleteTree)(ContactTree * root) = dlsym(library, "deleteTree");

    printf("Delete tree\n");
    root = deleteTree(root);
    printTimes(&previousTms, &previousClock,false);


}

