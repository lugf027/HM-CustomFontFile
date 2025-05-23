export const add: (a: number, b: number) => number;

type XComponentContextStatus = {
  hasDraw: boolean,
  hasChangeColor: boolean,
};

export const SetSurfaceId: (id: BigInt) => any;
export const ChangeSurface: (id: BigInt, w: number, h: number) =>any;
export const DrawWithCustomFont: (id: BigInt, fontPath: string, content: string, fontHeight: number) => any;
export const GetXComponentStatus: (id: BigInt) => XComponentContextStatus;
export const DestroySurface: (id: BigInt) => any;
