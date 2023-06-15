import Classificator


def teller(current, total, is_dup, filename, new_filename):

    dup_info = ''
    if is_dup:
        dup_info = ', Duplicated found, new filename: %s' % new_filename

    print('(%f)%% current File: %s %s' %
          ((current/total*100), filename, dup_info))


duplicated = Classificator.Classificate(
    '../SampleFileGen/GenPics', '../rst', teller)

print(duplicated)
