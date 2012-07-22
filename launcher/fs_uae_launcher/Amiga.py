from __future__ import division
from __future__ import print_function
from __future__ import absolute_import

A500_KICKSTARTS = [
    # amiga-os-130.rom (decrypted)
    # Kickstart v1.3 r34.5 (1987)(Commodore)(A3000)[!]
    "c39bd9094d4e5f4e28c1411f3086950406062e87",
    # Kickstart v1.3 r34.5 (1987)(Commodore)(A500-A1000-A2000-CDTV)[!]
    "891e9a547772fe0c6c19b610baf8bc4ea7fcb785",
    # Kickstart v1.3 r34.5 (1987)(Commodore)(A500-A1000-A2000-CDTV)[o]
    "90933936cce43ca9bc6bf375662c076b27e3c458"
]

A500P_KICKSTARTS = [
    # amiga-os-204.rom (decrypted)
    # Kickstart v2.04 r37.175 (1991)(Commodore)(A500+)[!]
    "c5839f5cb98a7a8947065c3ed2f14f5f42e334a1",
]

A600_KICKSTARTS = [
    # amiga-os-205.rom (decrypted)
    # Kickstart v2.05 r37.350 (1992)(Commodore)(A600HD)[!]
    "02843c4253bbd29aba535b0aa3bd9a85034ecde4",
    # Kickstart v2.05 r37.299 (1991)(Commodore)(A600)[!]
    "87508de834dc7eb47359cede72d2e3c8a2e5d8db",
]

A1200_KICKSTARTS = [
    # amiga-os-310-a1200.rom (decrypted)
    # Kickstart v3.1 r40.68 (1993)(Commodore)(A1200)[!]
    "e21545723fe8374e91342617604f1b3d703094f1",
]

A4000_KICKSTARTS = [
    # amiga-os-310.rom (decrypted)
    # Kickstart v3.1 r40.68 (1993)(Commodore)(A4000)[h Cloanto]
    "c3c481160866e60d085e436a24db3617ff60b5f9",
    # Kickstart v3.1 r40.68 (1993)(Commodore)(A4000)
    "5fe04842d04a489720f0f4bb0e46948199406f49",
]

CD32_KICKSTARTS = [
    # amiga-os-310-cd32.rom (decrypted)
    # Kickstart v3.1 r40.60 (1993)(Commodore)(CD32)
    "3525be8887f79b5929e017b42380a79edfee542d",
]

CDTV_KICKSTARTS = A500_KICKSTARTS

class Amiga:

    models = [
        {
            #"title": "Amiga 500",
            "title": "A500",
            "cd_based": False,
            "kickstarts": A500_KICKSTARTS,
        }, {
            #"title": "Amiga 500+",
            "title": "A500+",
            "cd_based": False,
            "kickstarts": A500P_KICKSTARTS,
        }, {
            #"title": "Amiga 600",
            "title": "A600",
            "cd_based": False,
            "kickstarts": A600_KICKSTARTS,
        }, {
            #"title": "Amiga 1200",
            "title": "A1200",
            "cd_based": False,
            "kickstarts": A1200_KICKSTARTS,
        }, {
            #"title": "Amiga 1200/020",
            "title": "A1200/020",
            "cd_based": False,
            "kickstarts": A1200_KICKSTARTS,
        }, {
            #"title": "Amiga 4000/040",
            "title": "A4000/040",
            "cd_based": False,
            "kickstarts": A4000_KICKSTARTS,
        }, {
            #"title": "Amiga CD32",
            "title": "CD32",
            "cd_based": True,
            "kickstarts": CD32_KICKSTARTS,
        }, {
            #"title": "Commodore CDTV",
            "title": "CDTV",
            "cd_based": True,
            "kickstarts": CDTV_KICKSTARTS,
        }
    ]

    models_config = [
        "A500",
        "A500+",
        "A600",
        "A1200",
        "A1200/020",
        "A4000/040",
        "CD32",
        "CDTV",
    ]

    @classmethod
    def is_cd_based(cls):
        return cls.get_current_config()["cd_based"]

    @classmethod
    def get_current_config(cls):
        from .Config import Config
        return cls.get_model_config(Config.get("amiga_model"))

    @classmethod
    def get_model_config(cls, model):
        print("get_model_config for", model)
        for i in range(len(cls.models_config)):
            if cls.models_config[i] == model:
                return cls.models[i]
        return {
            "title": "Dummy",
            "cd_based": False,
            "kickstarts": [
            ],
        }
