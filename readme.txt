Turcu Alexandru, grupa 333AA (fac laboratorul cu grupa 335CC)

La crearea threadurilor in main, am folosit ca parametru functia run_genetic_algorithm. Deoarece functia trebuie
ca acum sa aiba doar parametrul *arg, am adaugat intr-o structura in genetic_algorithm.h parametrii initiali.
Pe langa acestia (objects, object_count, generations_count si sack_capacity), structura mai contine ID-ul
threadului curent, numarul de threaduri, bariera, current_generation si next_generation. In run_genetic_algorithm
am atribuit parametrului *arg o structura numita argum. Inainte de crearea thread-urilor, am initializat
elementele structurii.

In cadrul functiei run_genetic_algorithm am paralelizat numeroase portiuni : calcularea fitness-ului, sortarea
(am folosit algoritmul OETS pentru scalabilitate), selectia elitei, mutatiile bit string, crossover-ul si
trecerea la urmatoarea generatie. Pentru fiecare secventa am calculat start si end (in functia compute_fitness_function
pentru calcularea fitness-ului si in main pentru celelalte operatii) pentru a imparti vectorii in portiuni pe care
itereaza cate un thread, apoi am pus bariera. De asemenea, am introdus numarul de thread-uri in functia read_input
pentru a putea citi numarul de thread-uri pentru care doresc sa fac comparatia dintre implementarea mea si cea
secventiala.