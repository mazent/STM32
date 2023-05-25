import sys

if __name__ == '__main__':
    if len(sys.argv) != 2:
        print('passare il nome del file')
    else:
        NOMEF = sys.argv[1]

        pbuf = {}

        conta = 0

        with open(NOMEF, 'rt') as ing:
            while True:
                riga = ing.readline()
                if len(riga) == 0:
                    break
                conta += 1

                if 'HAL_ETH_RxAllocateCallback' in riga:
                    elem = riga.strip().split(' ')
                    if len(elem) == 3:
                        if elem[0] not in pbuf:
                            pbuf[elem[0]] = 1
                        else:
                            pbuf[elem[0]] += 1
                    else:
                        print('{}) {} invece di 3'.format(conta, len(elem)))
                    continue

                if 'pbuf_free_custom' in riga:
                    elem = riga.strip().split(' ')
                    if len(elem) == 2:
                        if elem[1] not in pbuf:
                            pbuf[elem[1]] = 0
                        else:
                            pbuf[elem[1]] -= 1
                    else:
                        print('{}) {} invece di 2'.format(conta, len(elem)))
                    continue

        for k, v in pbuf.items():
            print('{} = {}'.format(k, v))
